//#define BOOST_NO_SCOPED_ENUMS
#include <fcl/core/filesystem.hpp>
#include <fcl/exception/exception.hpp>
#include <fcl/core/fwd_impl.hpp>
#include <fcl/core/utility.hpp>

#include <fcl/core/utf8.hpp>
#include <fcl/variant/variant.hpp>

#include <boost/config.hpp>

#include <fstream>

#ifdef _WIN32
# include <windows.h>
# include <userenv.h>
# include <shlobj.h>
#else
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <pwd.h>
# ifdef FCL_HAS_SIMPLE_FILE_LOCK
  #include <sys/file.h>
  #include <fcntl.h>
# endif
#endif

namespace fcl {
  // when converting to and from a variant, store utf-8 in the variant
  void to_variant( const std::filesystem::path& path_to_convert, variant& variant_output )
  {
    std::wstring wide_string = path_to_convert.generic_wstring();
    std::string utf8_string;
    fcl::encodeUtf8(wide_string, &utf8_string);
    variant_output = utf8_string;

    //std::string path = t.to_native_ansi_path();
    //std::replace(path.begin(), path.end(), '\\', '/');
    //v = path;
  }

  void from_variant( const fcl::variant& variant_to_convert, std::filesystem::path& path_output )
  {
    std::wstring wide_string;
    fcl::decodeUtf8(variant_to_convert.as_string(), &wide_string);
    path_output = std::filesystem::path(wide_string);
  }

   const std::filesystem::path& home_path()
   {
      static std::filesystem::path p = []()
      {
#ifdef WIN32
          HANDLE access_token;
          if (!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &access_token))
            FCL_ASSERT(false, "Unable to open an access token for the current process");
          wchar_t user_profile_dir[MAX_PATH];
          DWORD user_profile_dir_len = sizeof(user_profile_dir);
          BOOL success = GetUserProfileDirectoryW(access_token, user_profile_dir, &user_profile_dir_len);
          CloseHandle(access_token);
          if (!success)
            FCL_ASSERT(false, "Unable to get the user profile directory");
          return std::filesystem::path(std::wstring(user_profile_dir));
#else
          char* home = getenv( "HOME" );
          if( nullptr == home )
          {
             struct passwd* pwd = getpwuid(getuid());
             if( pwd )
             {
                 return std::filesystem::path( std::string( pwd->pw_dir ) );
             }
             FCL_ASSERT( home != nullptr, "The HOME environment variable is not set" );
          }
          return std::filesystem::path( std::string(home) );
#endif
      }();
      return p;
   }

   const std::filesystem::path& app_path()
   {
#ifdef __APPLE__
         static std::filesystem::path appdir = [](){  return home_path() / "Library" / "Application Support"; }();
#elif defined( WIN32 )
         static std::filesystem::path appdir = [](){
           wchar_t app_data_dir[MAX_PATH];

           if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, app_data_dir)))
             FCL_ASSERT(false, "Unable to get the current AppData directory");
           return std::filesystem::path(std::wstring(app_data_dir));
         }();
#else
        static std::filesystem::path appdir = home_path() / ".local/share";
#endif
      return appdir;
   }

}
