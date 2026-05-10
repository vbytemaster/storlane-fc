#include <boost/test/unit_test.hpp>

#include <fcl/json/json.hpp>
#include <fcl/exception/exception.hpp>

using namespace fcl;

BOOST_AUTO_TEST_SUITE(json_test_suite)

namespace json_test_util {
   constexpr size_t exception_limit_size = 250;

   const json::yield_function_t yield_deadline_exception_at_start = [](size_t s) {
      FCL_CHECK_DEADLINE(fcl::time_point::now() - fcl::milliseconds(1));
   };

   const json::yield_function_t yield_deadline_exception_in_mid = [](size_t s) {
      if (s >= exception_limit_size) {
         throw fcl::timeout_exception(fcl::exception_code::timeout_exception_code, "timeout_exception", "execution timed out" );
      }
   };

   const json::yield_function_t yield_length_exception = [](size_t s) {
      FCL_ASSERT( s <= exception_limit_size );
   };

   const json::yield_function_t yield_no_limitation = [](size_t s) {
      // no limitation
   };

   const auto time_except_verf_func = [](const fcl::timeout_exception& ex) -> bool {
      return (static_cast<int>(ex.code_value) == static_cast<int>(fcl::exception_code::timeout_exception_code));
   };

   const auto length_limit_except_verf_func = [](const fcl::assert_exception& ex) -> bool {
      return (static_cast<int>(ex.code_value) == static_cast<int>(fcl::exception_code::assert_exception_code));
   };

   constexpr size_t repeat_char_num = 512;
   const std::string repeat_chars(repeat_char_num, 'a');
   const std::string escape_input_str = "\\b\\f\\n\\r\\t-\\-\\\\-\\x0\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x08\\x09\\x0a\\x0b\\x0c\\x0d\\x0e\\x0f"  \
                                   "\\x10\\x11\\x12\\x13\\x14\\x15\\x16\\x17\\x18\\x19\\x1a\\x1b\\x1c\\x1d\\x1e\\x1f-" + repeat_chars;

}  // namespace json_test_util

BOOST_AUTO_TEST_CASE(to_string_test)
{
   {  // to_string( const variant& v, const fcl::time_point& deadline, const uint64_t max_len = max_length_limit, output_formatting format = stringify_large_ints_and_doubles);
      {
         variant v(json_test_util::escape_input_str);
         std::string deadline_exception_at_start_str;
         BOOST_CHECK_EXCEPTION(deadline_exception_at_start_str = json::to_string( v, fcl::time_point::min(), json::output_formatting::stringify_large_ints_and_doubles, json::max_length_limit),
                               fcl::timeout_exception,
                               json_test_util::time_except_verf_func);
         BOOST_CHECK_EQUAL(deadline_exception_at_start_str.empty(), true);
      }
      {
         constexpr size_t max_len = 10;
         variant v(json_test_util::repeat_chars);
         std::string deadline_exception_at_start_str;
         BOOST_CHECK_EQUAL(deadline_exception_at_start_str.empty(), true);
         BOOST_CHECK_EXCEPTION(deadline_exception_at_start_str = json::to_string( v, fcl::time_point::maximum(), json::output_formatting::stringify_large_ints_and_doubles, max_len),
                               fcl::assert_exception,
                               json_test_util::length_limit_except_verf_func);
         BOOST_CHECK_EQUAL(deadline_exception_at_start_str.empty(), true);
      }
      {
         variant v(json_test_util::repeat_chars);
         std::string length_exception_in_mid_str;
         BOOST_CHECK_NO_THROW(length_exception_in_mid_str = json::to_string( v, fcl::time_point::maximum(), json::output_formatting::stringify_large_ints_and_doubles, json::max_length_limit));
         BOOST_CHECK_EQUAL(length_exception_in_mid_str, "\"" + json_test_util::repeat_chars + "\"");
      }
      {
         variant v(4294967296LL); // 0xFFFFFFFF + 1
         std::string large_int = json::to_string( v, fcl::time_point::maximum(), json::output_formatting::stringify_large_ints_and_doubles, json::max_length_limit);
         BOOST_CHECK_EQUAL(large_int, "\"4294967296\"");

         variant v1(4294967295LL); // 0xFFFFFFFF
         std::string normal_int = json::to_string( v1, fcl::time_point::maximum(), json::output_formatting::stringify_large_ints_and_doubles, json::max_length_limit);
         BOOST_CHECK_EQUAL(normal_int, "4294967295");

         variant v2(-4294967296LL);
         std::string large_int_neg = json::to_string( v2, fcl::time_point::maximum(), json::output_formatting::stringify_large_ints_and_doubles, json::max_length_limit);
         BOOST_CHECK_EQUAL(large_int_neg, "\"-4294967296\"");

         variant v3(-4294967295LL);
         std::string normal_int_neg = json::to_string( v3, fcl::time_point::maximum(), json::output_formatting::stringify_large_ints_and_doubles, json::max_length_limit);
         BOOST_CHECK_EQUAL(normal_int_neg, "-4294967295");

         variant v4(-90909090909090909LL);
         std::string super_neg = json::to_string( v4, fcl::time_point::maximum(), json::output_formatting::stringify_large_ints_and_doubles, json::max_length_limit);
         BOOST_CHECK_EQUAL(super_neg, "\"-90909090909090909\"");
      }
   }
   {  // to_string( const variant& v, const yield_function_t& yield, output_formatting format = stringify_large_ints_and_doubles);
      const variant v(json_test_util::repeat_chars);
      BOOST_CHECK_LT(json_test_util::exception_limit_size, json_test_util::repeat_chars.size());
      {
         std::string deadline_exception_at_start_str;
         BOOST_CHECK_EXCEPTION(deadline_exception_at_start_str = json::to_string( v, json_test_util::yield_deadline_exception_at_start),
                               fcl::timeout_exception,
                               json_test_util::time_except_verf_func);
         BOOST_CHECK_EQUAL(deadline_exception_at_start_str.empty(), true);
      }
      {
         std::string deadline_exception_in_mid_str;
         BOOST_CHECK_EXCEPTION(deadline_exception_in_mid_str = json::to_string( v, json_test_util::yield_deadline_exception_in_mid),
                               fcl::timeout_exception,
                               json_test_util::time_except_verf_func);
         BOOST_CHECK_EQUAL(deadline_exception_in_mid_str.empty(), true);
      }
      {
         std::string length_exception_in_mid_str;
         BOOST_CHECK_EXCEPTION(length_exception_in_mid_str = json::to_string( v, json_test_util::yield_length_exception),
                               fcl::assert_exception,
                               json_test_util::length_limit_except_verf_func);
         BOOST_CHECK_EQUAL(length_exception_in_mid_str.empty(), true);
      }
      {
         std::string no_exception_str;
         BOOST_CHECK_NO_THROW(no_exception_str = json::to_string( v, json_test_util::yield_no_limitation));
         BOOST_CHECK_EQUAL(no_exception_str, "\"" + json_test_util::repeat_chars + "\"");
      }
   }
   { // to_string template call
      const uint16_t id = 1000;
      const uint64_t len = 3;
      const std::string id_ret_1 = json::to_string( id, fcl::time_point::maximum());
      BOOST_CHECK_EQUAL(std::to_string(id), id_ret_1);

      // exceed length
      std::string id_ret_2;
      BOOST_REQUIRE_THROW(id_ret_2 = json::to_string( id, fcl::time_point::maximum(), json::output_formatting::stringify_large_ints_and_doubles, len), fcl::assert_exception);
      BOOST_CHECK_EQUAL(id_ret_2.empty(), true);

      // time_out
      std::string id_ret_3;
      BOOST_REQUIRE_THROW(id_ret_3 = json::to_string( id, fcl::time_point::now() - fcl::milliseconds(1) ), fcl::timeout_exception);
      BOOST_CHECK_EQUAL(id_ret_3.empty(), true);
   }
}

BOOST_AUTO_TEST_CASE(to_pretty_string_test)
{
   {  // to_pretty_string( const variant& v, const fcl::time_point& deadline, const uint64_t max_len = max_length_limit, output_formatting format = stringify_large_ints_and_doubles );
      {
         variant v(json_test_util::escape_input_str);
         std::string deadline_exception_at_start_str;
         BOOST_CHECK_EXCEPTION(deadline_exception_at_start_str = json::to_pretty_string( v, fcl::time_point::min(), json::output_formatting::stringify_large_ints_and_doubles, json::max_length_limit),
         fcl::timeout_exception,
         json_test_util::time_except_verf_func);
         BOOST_CHECK_EQUAL(deadline_exception_at_start_str.empty(), true);
      }
      {
         constexpr size_t max_len = 10;
         variant v(json_test_util::repeat_chars);
         std::string deadline_exception_at_start_str;
         BOOST_CHECK_EQUAL(deadline_exception_at_start_str.empty(), true);
         BOOST_CHECK_EXCEPTION(deadline_exception_at_start_str = json::to_pretty_string( v, fcl::time_point::maximum(), json::output_formatting::stringify_large_ints_and_doubles, max_len),
                               fcl::assert_exception,
                               json_test_util::length_limit_except_verf_func);
         BOOST_CHECK_EQUAL(deadline_exception_at_start_str.empty(), true);
      }
      {
         variant v(json_test_util::repeat_chars);
         std::string length_exception_in_mid_str;
         BOOST_CHECK_NO_THROW(length_exception_in_mid_str = json::to_pretty_string( v, fcl::time_point::maximum(), json::output_formatting::stringify_large_ints_and_doubles, json::max_length_limit));
         BOOST_CHECK_EQUAL(length_exception_in_mid_str, "\"" + json_test_util::repeat_chars + "\"");
      }
   }
   {  // to_pretty_string( const variant& v, const yield_function_t& yield, output_formatting format = stringify_large_ints_and_doubles );
      const variant v(json_test_util::repeat_chars);
      BOOST_CHECK_LT(json_test_util::exception_limit_size, json_test_util::repeat_chars.size());
      {
         std::string deadline_exception_at_start_str;
         BOOST_CHECK_EXCEPTION(deadline_exception_at_start_str = json::to_pretty_string( v, json_test_util::yield_deadline_exception_at_start),
                               fcl::timeout_exception,
                               json_test_util::time_except_verf_func);
         BOOST_CHECK_EQUAL(deadline_exception_at_start_str.empty(), true);
      }
      {
         std::string deadline_exception_in_mid_str;
         BOOST_CHECK_EXCEPTION(deadline_exception_in_mid_str = json::to_pretty_string( v, json_test_util::yield_deadline_exception_in_mid),
                               fcl::timeout_exception,
                               json_test_util::time_except_verf_func);
         BOOST_CHECK_EQUAL(deadline_exception_in_mid_str.empty(), true);
      }
      {
         std::string length_exception_in_mid_str;
         BOOST_CHECK_EXCEPTION(length_exception_in_mid_str = json::to_pretty_string( v, json_test_util::yield_length_exception),
                               fcl::assert_exception,
                               json_test_util::length_limit_except_verf_func);
         BOOST_CHECK_EQUAL(length_exception_in_mid_str.empty(), true);
      }
      {
         std::string no_exception_str;
         BOOST_CHECK_NO_THROW(no_exception_str = json::to_pretty_string( v, json_test_util::yield_no_limitation));
         BOOST_CHECK_EQUAL(no_exception_str, "\"" + json_test_util::repeat_chars + "\"");
      }
   }
   { // to_pretty_string template call
      const uint16_t id = 1000;
      const uint64_t len = 3;
      const std::string id_ret_1 = json::to_pretty_string( id, fcl::time_point::maximum());
      BOOST_CHECK_EQUAL(std::to_string(id), id_ret_1);

      // exceed length
      std::string id_ret_2;
      BOOST_REQUIRE_THROW(id_ret_2 = json::to_pretty_string( id, fcl::time_point::maximum(), json::output_formatting::stringify_large_ints_and_doubles, len), fcl::assert_exception);
      BOOST_CHECK_EQUAL(id_ret_2.empty(), true);

      // time_out
      std::string id_ret_3;
      BOOST_REQUIRE_THROW(id_ret_3 = json::to_pretty_string( id, fcl::time_point::now() - fcl::milliseconds(1) ), fcl::timeout_exception);
      BOOST_CHECK_EQUAL(id_ret_3.empty(), true);
   }
}

BOOST_AUTO_TEST_CASE(escape_string_test)
{
   std::string escape_out_str;
   escape_out_str = fcl::escape_string(json_test_util::escape_input_str, json_test_util::yield_no_limitation);
   BOOST_CHECK_LT(json_test_util::repeat_char_num, json_test_util::escape_input_str.size());
   BOOST_CHECK_LT(json_test_util::escape_input_str.size() - json_test_util::repeat_char_num, json_test_util::exception_limit_size);  // by using size_different to calculate expected string
   {
      // simulate exceed time exception at the beginning of processing
      BOOST_CHECK_EXCEPTION(escape_string(json_test_util::escape_input_str, json_test_util::yield_deadline_exception_at_start),
                            fcl::timeout_exception,
                            json_test_util::time_except_verf_func);
   }
   {
      // simulate exceed time exception in the middle of processing
      BOOST_CHECK_EXCEPTION(escape_string(json_test_util::escape_input_str, json_test_util::yield_deadline_exception_in_mid),
                            fcl::timeout_exception,
                            json_test_util::time_except_verf_func);
   }
   {
      // length limitation exception in the middle of processing
      BOOST_CHECK_EXCEPTION(escape_string(json_test_util::escape_input_str, json_test_util::yield_length_exception),
                            fcl::assert_exception,
                            json_test_util::length_limit_except_verf_func);
   }
}

BOOST_AUTO_TEST_SUITE_END()
