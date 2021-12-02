// Copyright (c) 2017-2021, Lawrence Livermore National Security, LLC and
// other Axom Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

#ifndef AXOM_ARRAYVIEW_HPP_
#define AXOM_ARRAYVIEW_HPP_

#include "axom/core/memory_management.hpp"  // for memory allocation functions
#include "axom/core/ArrayBase.hpp"
#include "axom/core/ArrayIteratorBase.hpp"

namespace axom
{
// Forward declare the templated classes and operator function(s)
template <typename T, int DIM, MemorySpace SPACE>
class ArrayView;

template <typename T, int DIM, MemorySpace SPACE>
struct ArrayTraits<ArrayView<T, DIM, SPACE>>
{
    constexpr static bool IsView = true;
};

/// \name ArrayView to wrap a pointer and provide indexing semantics
/// @{

/*!
 * \class ArrayView
 *
 * \brief Provides a view over a generic array container.
 * 
 * The ArrayView expresses a non-owning relationship over a pointer
 *
 * \tparam T the type of the values to hold.
 * \tparam DIM The dimension of the array.
 *
 */
template <typename T, int DIM = 1, MemorySpace SPACE = MemorySpace::Dynamic>
class ArrayView : public ArrayBase<T, DIM, ArrayView<T, DIM, SPACE>>
{
public:
  using value_type = T;
  static constexpr int dimension = DIM;
  static constexpr MemorySpace space = SPACE;
  using ArrayViewIterator = ArrayIteratorBase<ArrayView<T, DIM, SPACE>, T>;

  /// \brief Default constructor
  ArrayView() : m_allocator_id(axom::detail::getAllocatorID<SPACE>()) { }

  /*!
   * \brief Generic constructor for an ArrayView of arbitrary dimension with external data
   *
   * \param [in] data the external data this ArrayView will wrap.
   * \param [in] args The parameter pack containing the "shape" of the ArrayView
   *
   * \pre sizeof...(Args) == DIM
   *
   * \post size() == num_elements
   */
  template <typename... Args>
  ArrayView(T* data, Args... args);

  /*! 
   * \brief Constructor for transferring between memory spaces
   * 
   * \param [in] other The array in a different memory space to copy from
   * 
   * \note The parameter is non-const because \a other can be modified through the constructed View
   * 
   * \note This constructor is left implicit to allow for convenient function calls that convert
   * from \p Array -> \p ArrayView or from dynamic memory spaces to an \p ArrayView of explicitly specified
   * space.
   */
  template <typename OtherArrayType>
  ArrayView(ArrayBase<T, DIM, OtherArrayType>& other);
  /// \overload
  template <typename OtherArrayType>
  ArrayView(
    const ArrayBase<typename std::remove_const<T>::type, DIM, OtherArrayType>& other);

  /*!
   * \brief Return the number of elements stored in the data array.
   */
  inline AXOM_HOST_DEVICE IndexType size() const { return m_num_elements; }

  /*!
   * \brief Returns an ArrayViewIterator to the first element of the Array
   */
  ArrayViewIterator begin() const
  {
    assert(m_data != nullptr);
    return ArrayViewIterator(0, this);
  }

  /*!
   * \brief Returns an ArrayViewIterator to the element following the last
   *  element of the Array.
   */
  ArrayViewIterator end() const
  {
    assert(m_data != nullptr);
    return ArrayViewIterator(size(), this);
  }

  /*!
   * \brief Return a pointer to the array of data.
   */
  /// @{

  AXOM_HOST_DEVICE inline T* data() const
  {
    return m_data;
  }

  /// @}

  /*!
   * \brief Get the ID for the umpire allocator
   */
  int getAllocatorID() const { return m_allocator_id; }

private:
  T* m_data = nullptr;
  /// \brief The full number of elements in the array
  ///  i.e., 3 for a 1D Array of size 3, 9 for a 3x3 2D array, etc
  IndexType m_num_elements = 0;
  /// \brief The allocator ID for the memory space in which m_data was allocated
  int m_allocator_id;
};

/// \brief Helper alias for multi-component arrays
template <typename T>
using MCArrayView = ArrayView<T, 2>;

//------------------------------------------------------------------------------
//                            ArrayView IMPLEMENTATION
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <typename T, int DIM, MemorySpace SPACE>
template <typename... Args>
ArrayView<T, DIM, SPACE>::ArrayView(T* data, Args... args)
  : ArrayBase<T, DIM, ArrayView<T, DIM, SPACE>>(args...)
  , m_data(data)
  , m_allocator_id(axom::detail::getAllocatorID<SPACE>())
{
  static_assert(sizeof...(Args) == DIM,
                "Array size must match number of dimensions");
#ifdef AXOM_DEVICE_CODE
    static_assert((SPACE == MemorySpace::Constant)
                  == std::is_const<T>::value,
                  "T must be const if memory space is Constant memory");
#endif
  // Intel hits internal compiler error when casting as part of function call
  IndexType tmp_args[] = {args...};
  m_num_elements = detail::packProduct(tmp_args);

#ifdef AXOM_USE_UMPIRE
  // If we have Umpire, we can try and see what space the pointer is allocated in
  // Probably not worth checking this if SPACE != Dynamic, we *could* error out
  // if e.g., the user gives a host pointer to ArrayView<T, DIM, Device>, but even
  // Thrust doesn't guard against this.

  // FIXME: Is it worth trying to get rid of this at compile time?
  // (using a workaround since we don't have "if constexpr")
  if(SPACE == MemorySpace::Dynamic)
  {
    auto& rm = umpire::ResourceManager::getInstance();

    using NonConstT = typename std::remove_const<T>::type;
    // TODO: There's no reason these Umpire methods should take a non-const pointer.
    if(rm.hasAllocator(const_cast<NonConstT*>(data)))
    {
      auto alloc = rm.getAllocator(const_cast<NonConstT*>(data));
      m_allocator_id = alloc.getId();
    }
  }
#endif
}

//------------------------------------------------------------------------------
template <typename T, int DIM, MemorySpace SPACE>
template <typename OtherArrayType>
ArrayView<T, DIM, SPACE>::ArrayView(ArrayBase<T, DIM, OtherArrayType>& other)
  : ArrayBase<T, DIM, ArrayView<T, DIM, SPACE>>(other)
  , m_data(static_cast<OtherArrayType&>(other).data())
  , m_num_elements(static_cast<OtherArrayType&>(other).size())
  , m_allocator_id(static_cast<OtherArrayType&>(other).getAllocatorID())
{
#ifdef AXOM_DEBUG
  // If it's not dynamic, the allocator ID from the argument array has to match the template param.
  // If that's not the case then things have gone horribly wrong somewhere.
  if(SPACE != MemorySpace::Dynamic &&
     m_allocator_id != axom::detail::getAllocatorID<SPACE>())
  {
    std::cerr << "Input argument allocator does not match the explicitly "
                 "provided memory space\n";
    utilities::processAbort();
  }
#endif
}

//------------------------------------------------------------------------------
template <typename T, int DIM, MemorySpace SPACE>
template <typename OtherArrayType>
ArrayView<T, DIM, SPACE>::ArrayView(
  const ArrayBase<typename std::remove_const<T>::type, DIM, OtherArrayType>& other)
  : ArrayBase<T, DIM, ArrayView<T, DIM, SPACE>>(other)
  , m_data(static_cast<const OtherArrayType&>(other).data())
  , m_num_elements(static_cast<const OtherArrayType&>(other).size())
  , m_allocator_id(static_cast<const OtherArrayType&>(other).getAllocatorID())
{
  static_assert(
    std::is_const<T>::value,
    "Cannot create an ArrayView of non-const type from a const Array");
#ifdef AXOM_DEBUG
  // If it's not dynamic, the allocator ID from the argument array has to match the template param.
  // If that's not the case then things have gone horribly wrong somewhere.
  if(SPACE != MemorySpace::Dynamic &&
     m_allocator_id != axom::detail::getAllocatorID<SPACE>())
  {
    std::cerr << "Input argument allocator does not match the explicitly "
                 "provided memory space\n";
    utilities::processAbort();
  }
#endif
}

} /* namespace axom */

#endif /* AXOM_ARRAYVIEW_HPP_ */
