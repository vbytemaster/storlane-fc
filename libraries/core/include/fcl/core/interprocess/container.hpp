#pragma once

#include <fcl/core/container/container_detail.hpp>
#include <fcl/core/container/flat.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/set.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/flat_set.hpp>
#include <boost/interprocess/containers/flat_map.hpp>

namespace fcl {

   namespace bip = boost::interprocess;

   namespace raw {

      template<typename Stream, typename T, typename... U>
      void pack( Stream& s, const bip::set<T, U...>& value ) {
         detail::pack_set( s, value );
      }

      template<typename Stream, typename T, typename... U>
      void unpack( Stream& s, bip::set<T, U...>& value ) {
         detail::unpack_set( s, value );
      }

      template<typename Stream, typename T, typename... U>
      void pack( Stream& s, const bip::multiset<T, U...>& value ) {
         detail::pack_set( s, value );
      }

      template<typename Stream, typename T, typename... U>
      void unpack( Stream& s, bip::multiset<T, U...>& value ) {
         detail::unpack_set( s, value );
      }

      template<typename Stream, typename K, typename V, typename... U>
      void pack( Stream& s, const bip::map<K, V, U...>& value ) {
         detail::pack_map( s, value );
      }

      template<typename Stream, typename K, typename V, typename... U>
      void unpack( Stream& s, bip::map<K, V, U...>& value ) {
         detail::unpack_map( s, value );
      }

      template<typename Stream, typename K, typename V, typename... U>
      void pack( Stream& s, const bip::multimap<K, V, U...>& value ) {
         detail::pack_map( s, value );
      }

      template<typename Stream, typename K, typename V, typename... U>
      void unpack( Stream& s, bip::multimap<K, V, U...>& value ) {
         detail::unpack_map( s, value );
      }

   }

   template<typename T, typename... U>
   void to_variant( const bip::set< T, U... >& s, fcl::variant& vo ) {
      detail::to_variant_from_set( s, vo );
   }

   template<typename T, typename... U>
   void from_variant( const fcl::variant& v, bip::set< T, U... >& s ) {
      detail::from_variant_to_set( v, s );
   }

   template<typename T, typename... U>
   void to_variant( const bip::multiset< T, U... >& s, fcl::variant& vo ) {
      detail::to_variant_from_set( s, vo );
   }

   template<typename T, typename... U>
   void from_variant( const fcl::variant& v, bip::multiset< T, U... >& s ) {
      detail::from_variant_to_set( v, s );
   }

   template<typename K, typename V, typename... U >
   void to_variant( const bip::map< K, V, U... >& m, fcl::variant& vo ) {
      detail::to_variant_from_map( m, vo );
   }

   template<typename K, typename V, typename... U>
   void from_variant( const variant& v,  bip::map<K, V, U...>& m ) {
      detail::from_variant_to_map( v, m );
   }

   template<typename K, typename V, typename... U >
   void to_variant( const bip::multimap< K, V, U... >& m, fcl::variant& vo ) {
      detail::to_variant_from_map( m, vo );
   }

   template<typename K, typename V, typename... U>
   void from_variant( const variant& v,  bip::multimap<K, V, U...>& m ) {
      detail::from_variant_to_map( v, m );
   }

}
