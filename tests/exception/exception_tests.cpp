#include <boost/test/unit_test.hpp>
#include <fcl/exception/macros.hpp>

#include <chrono>
#include <source_location>
#include <stdexcept>
#include <string>
#include <system_error>

import fcl.exception.exception;

BOOST_AUTO_TEST_SUITE(exception_test_suite)

BOOST_AUTO_TEST_CASE(context_fields_are_formatted_and_redacted)
{
   const fcl::error::context_error error{
      "open vault",
      {fcl::error::ctx("path", "/tmp/fcl.vault"), fcl::error::secret("passphrase", "correct horse battery staple")},
      std::source_location::current()};

   const std::string text = error.what();
   BOOST_CHECK(text.find("open vault") != std::string::npos);
   BOOST_CHECK(text.find("path=/tmp/fcl.vault") != std::string::npos);
   BOOST_CHECK(text.find("passphrase=<redacted>") != std::string::npos);
   BOOST_CHECK(text.find("correct horse battery staple") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(context_error_keeps_optional_error_code)
{
   const fcl::error::context_error error{
      "deadline exceeded",
      {fcl::error::ctx("phase", "startup")},
      std::source_location::current(),
      std::make_error_code(std::errc::timed_out)};

   BOOST_CHECK_EQUAL(error.message(), "deadline exceeded");
   BOOST_CHECK_EQUAL(error.context().size(), 1u);
   BOOST_CHECK(error.code() == std::make_error_code(std::errc::timed_out));
   BOOST_CHECK(std::string(error.what()).find("timed out") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(capture_and_rethrow_preserves_nested_exception)
{
   try {
      try {
         throw std::runtime_error("inner failure");
      } FCL_CAPTURE_AND_RETHROW("outer context", fcl::error::ctx("phase", "initialize"))
   } catch (const fcl::error::context_error& error) {
      const auto chain = fcl::error::format_exception_chain(error);
      BOOST_CHECK(chain.find("outer context") != std::string::npos);
      BOOST_CHECK(chain.find("phase=initialize") != std::string::npos);
      BOOST_CHECK(chain.find("inner failure") != std::string::npos);
      return;
   }

   BOOST_FAIL("expected context_error");
}

BOOST_AUTO_TEST_CASE(assert_macro_throws_std_compatible_context_error)
{
   BOOST_CHECK_EXCEPTION(
      FCL_ASSERT(false, "broken invariant", fcl::error::ctx("value", 42)),
      fcl::error::context_error,
      [](const fcl::error::context_error& error) {
         const std::string text = error.what();
         return error.code() == std::make_error_code(std::errc::invalid_argument) &&
                text.find("broken invariant") != std::string::npos &&
                text.find("expression=false") != std::string::npos &&
                text.find("value=42") != std::string::npos;
      });
}

BOOST_AUTO_TEST_CASE(deadline_macro_throws_timeout_context_error)
{
   BOOST_CHECK_EXCEPTION(
      FCL_CHECK_DEADLINE(std::chrono::system_clock::now() - std::chrono::milliseconds(1), fcl::error::ctx("phase", "render")),
      fcl::error::context_error,
      [](const fcl::error::context_error& error) {
         return error.code() == std::make_error_code(std::errc::timed_out) &&
                std::string(error.what()).find("phase=render") != std::string::npos;
      });
}

BOOST_AUTO_TEST_SUITE_END()
