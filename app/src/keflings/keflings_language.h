// keflings-nx (2026-10-31, the user's request: the language in the toml): the game's language from the system's, for
// keflings_language = "" (from bejeweledblitz-nx's bejeweledblitz_language: the Switch's system language, or
// Windows' display language on the PC).
#pragma once

#include <string>

namespace keflings::language {

// The system language as one of the game's eight (en, ja, de, fr, es, it, ko, zh): regional variants give their
// language (both Chinese scripts zh), any other language (Portuguese, Dutch, Russian) or a system language that cannot
// be read gives en. `system_name` receives the system's own name for it, for the log.
std::string FromSystem(std::string* system_name = nullptr);

}  // namespace keflings::language
