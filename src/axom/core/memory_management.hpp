/*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright (c) 2017-2018, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory
 *
 * LLNL-CODE-741217
 *
 * All rights reserved.
 *
 * This file is part of Axom.
 *
 * For details about use and distribution, please read axom/LICENSE.
 *
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#ifndef AXOM_MEMORYMANAGEMENT_HPP_
#define AXOM_MEMORYMANAGEMENT_HPP_

// Axom includes
#include "axom/config.hpp" // for AXOM compile-time definitions

// Umpire includes
#ifdef AXOM_USE_UMPIRE
#include "umpire/config.hpp"
#include "umpire/Allocator.hpp"
#include "umpire/ResourceManager.hpp"
#endif

// C/C++ includes
#include <cassert>  // for assert()
#include <cstdlib>  // for std::malloc, std::realloc, std::free

namespace axom
{

/*!
 * \brief Enumerates the available memory spaces on a given system.
 *
 * \note The number of memory spaces available depends on the target system and
 *  whether Axom is compiled with CUDA and Umpire. Specifically, HOST memory is
 *  the default and is always available. If CUDA and UMPIRE support is enabled
 *  the following memory spaces are also available:
 *    * HOST_PINNED
 *    * DEVICE
 *    * DEVICE_CONSTANT
 *    * UNIFIED_MEMORY
 *
 */
enum MemorySpace
{
  HOST,

#if defined(AXOM_USE_CUDA) && defined(AXOM_USE_UMPIRE)

#ifdef UMPIRE_ENABLE_PINNED
  HOST_PINNED,
#endif

#ifdef UMPIRE_ENABLE_DEVICE
  DEVICE,
  DEVICE_CONSTANT,
#endif

#ifdef UMPIRE_ENABLE_UM
  UNIFIED_MEMORY,
#endif

#endif /* defined(AXOM_USE_CUDA) && defined(AXOM_USE_UMPIRE) */

  NUM_MEMORY_SPACES   //!< NUM_MEMORY_SPACES
};

/// \name Internal Data Structures
/// @{

namespace internal
{

/*!
 * \brief Holds the value for the default memory space.
 */
static MemorySpace s_mem_space = HOST;

#ifdef AXOM_USE_UMPIRE

/*!
 * \brief Maps a MemorySpace enum to the corresponding Umpire resource type.
 */
static const int umpire_type[ ] =
{
  umpire::resource::Host,

#ifdef AXOM_USE_CUDA

#ifdef UMPIRE_ENABLE_PINNED
  umpire::resource::Pinned,
#endif

#ifdef UMPIRE_ENABLE_DEVICE
  umpire::resource::Device,
  umpire::resource::Constant,
#endif

#ifdef UMPIRE_ENABLE_UM
  umpire::resource::Unified,
#endif

#endif /* AXOM_USE_CUDA */

};

#endif /* AXOM_USE_UMPIRE */

} /* end namspace internal */

/// @}


/// \name Memory Management Routines
/// @{

/*!
 * \brief Sets the default memory space to use. Default is set to HOST
 * \param [in] spaceId ID of the memory space to use.
 */
inline void setDefaultMemorySpace( MemorySpace spaceId );

/*!
 * \brief Allocates a chunk of memory of type T.
 * \param [in] n the number of elements to allocate.
 * \param [in] spaceId the space where memory will be allocated (optional)
 *
 * \note The default memory space used is HOST.
 *
 * \tparam T the type of pointer returned.
 *
 * \return A pointer to the new allocation or a null pointer if allocation
 *  failed.
 *
 *  \pre spaceId >= 0 && spaceId < NUM_MEMORY_SPACES
 */
template < typename T >
inline T* alloc( std::size_t n, MemorySpace spaceId=internal::s_mem_space );

/*!
 * \brief Frees the chunk of memory pointed to by pointer.
 *
 * \param [in] pointer pointer to memory previously allocated with
 *  alloc or realloc or a null pointer.
 *
 * \post pointer == nullptr
 */
template < typename T >
inline void free( T*& pointer );

/*!
 * \brief Reallocates the chunk of memory pointed to by pointer.
 * \param [in] pointer pointer to memory previously allocated with
 *  alloc or realloc, or a null pointer.
 * \param [in] n the number of elements to allocate.
 * \tparam T the type pointer points to.
 * \return A pointer to the new allocation or a null pointer if allocation
 *  failed.
 */
template < typename T >
inline T* realloc( T* pointer, std::size_t n );

/// @}

//------------------------------------------------------------------------------
//                        IMPLEMENTATION
//------------------------------------------------------------------------------

inline void setDefaultMemorySpace( MemorySpace spaceId )
{
  internal::s_mem_space = spaceId;

#ifdef AXOM_USE_UMPIRE

  auto& rm = umpire::ResourceManager::getInstance();

  umpire::Allocator allocator =
      rm.getAllocator( ( internal::umpire_type[ spaceId ] ) );

  rm.setDefaultAllocator( allocator );

#endif

}

//------------------------------------------------------------------------------
template < typename T >
inline T* alloc( std::size_t n, MemorySpace spaceId )
{
  // sanity checks
  assert( "pre: invalid memory space request" &&
          (spaceId >= HOST) && (spaceId < NUM_MEMORY_SPACES) );

  const std::size_t numbytes = n * sizeof( T );
  T* ptr = nullptr;

#ifdef AXOM_USE_UMPIRE

  auto& rm = umpire::ResourceManager::getInstance();

  umpire::Allocator allocator =
      rm.getAllocator( internal::umpire_type[ spaceId ] );

  ptr = static_cast< T* >( allocator.allocate( numbytes )  );

#else

  ptr = static_cast< T* >( std::malloc( numbytes ) );

#endif

  return ptr;
}

//------------------------------------------------------------------------------
template < typename T >
inline void free( T*& pointer )
{

#ifdef AXOM_USE_UMPIRE

  auto& rm       = umpire::ResourceManager::getInstance();
  auto allocator = rm.getAllocator( pointer );
  allocator.deallocate( pointer );

#else

  std::free( pointer );

#endif

  pointer = nullptr;

}

//------------------------------------------------------------------------------
template < typename T >
inline T* realloc( T* pointer, std::size_t n )
{
  if ( n==0 )
  {
    axom::free( pointer );
    return nullptr;
  }

  const std::size_t numbytes = n * sizeof( T );

#ifdef AXOM_USE_UMPIRE

  auto& rm = umpire::ResourceManager::getInstance();
  pointer = static_cast< T* >( rm.reallocate( pointer, numbytes ) );

#else

  pointer = static_cast< T* >( std::realloc( pointer, numbytes ) );

#endif

  return pointer;
}

} // namespace axom

#endif /* AXOM_MEMORYMANAGEMENT_HPP_ */
