// Copyright (c) 2014-2018, The Monero Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#pragma once

#include <vector>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <string>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/integral_constant.hpp>

#include "Common/StringTools.h"
#include "Serialization/BinarySerializationTools.h"
#include "Common/hex_str.h"
#include "binary_archive.h"

namespace CryptoNote
{
typedef std::string blobdata;

namespace serial {

template <class T>
struct is_blob_type { typedef boost::false_type type; };

template <class T>
struct is_basic_type { typedef boost::false_type type; };

template<typename F, typename S>
struct is_basic_type<std::pair<F,S>> { typedef boost::true_type type; };
template<>
struct is_basic_type<std::string> { typedef boost::true_type type; };

template <class Archive, class T>
struct serializer{
  static bool serialize(Archive &ar, T &v) {
    return serialize(ar, v, typename boost::is_integral<T>::type(), typename is_blob_type<T>::type(), typename is_basic_type<T>::type());
  }
  template<typename A>
  static bool serialize(Archive &ar, T &v, boost::false_type, boost::true_type, A a) {
    ar.serialize_blob(&v, sizeof(v));
    return true;
  }
  template<typename A>
  static bool serialize(Archive &ar, T &v, boost::true_type, boost::false_type, A a) {
    ar.serialize_int(v);
    return true;
  }
  static bool serialize(Archive &ar, T &v, boost::false_type, boost::false_type, boost::true_type) {
    return do_serialize(ar, v);
  }
  static void serialize_custom(Archive &ar, T &v, boost::true_type) {
  }
};

template <class Archive, class T>
inline bool do_serialize(Archive &ar, T &v)
{
  return serializer<Archive, T>::serialize(ar, v);
}
template <class Archive>
inline bool do_serialize(Archive &ar, bool &v)
{
  ar.serialize_blob(&v, sizeof(v));
  return true;
}

namespace detail {
    /*! \fn do_check_stream_state
     *
     * \brief self explanatory
     */
    template<class Stream>
    bool do_check_stream_state(Stream& s, boost::mpl::bool_<true>)
    {
      return s.good();
    }
    /*! \fn do_check_stream_state
     *
     * \brief self explanatory
     *
     * \detailed Also checks to make sure that the stream is not at EOF
     */
    template<class Stream>
    bool do_check_stream_state(Stream& s, boost::mpl::bool_<false>)
    {
      bool result = false;
      if (s.good())
        {
          std::ios_base::iostate state = s.rdstate();
          result = EOF == s.peek();
          s.clear(state);
        }
      return result;
    }
  }

  /*! \fn check_stream_state
   *
   * \brief calls detail::do_check_stream_state for ar
   */
  template<class Archive>
  bool check_stream_state(Archive& ar)
  {
    return detail::do_check_stream_state(ar.stream(), typename Archive::is_saving());
  }

  /*! \fn serialize
   *
   * \brief serializes \a v into \a ar
   */
  template <class Archive, class T>
  inline bool serialize(Archive &ar, T &v)
  {
    bool r = do_serialize(ar, v);
    return r && check_stream_state(ar);
  }
}

  template<class t_object>
  bool t_serializable_object_to_blob(const t_object& to, CryptoNote::blobdata& b_blob)
  {
    std::stringstream ss;
    ss << b_blob;
    BinaryArray ba = hex_to_bin(ss.str());
//    bool r = serial::serialize(ba, const_cast<t_object&>(to));
    b_blob = ss.str();
    bool r = b_blob.empty();
    return !r;
  }
  //---------------------------------------------------------------
  template<class t_object>
  CryptoNote::BinaryArray t_serializable_object_to_blob(const t_object& to)
  {
    CryptoNote::blobdata b;
    t_serializable_object_to_blob(to, b);
    CryptoNote::BinaryArray ba = hex_to_bin(b);
    return ba;
  }
} // namespace CryptoNote

