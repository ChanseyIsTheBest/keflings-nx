// keflings-nx (2026-10-31): the console user's picture in the game's picture boxes (keflings_user_picture.cpp).
#pragma once

#include <cstdint>
#include <string>

namespace keflings::user_picture {

// Reads the picture of the console's user number `user` (1 = the first in the HOME Menu's order; a number past the
// last user takes the first) and gives it to XAM (rex/kernel/xam/gamer_picture.h). `file`: a JPEG or PNG that takes
// the console's picture's place (the PC has no console users). Returns a line for the log.
std::string Install(uint32_t user, const std::string& file);

}  // namespace keflings::user_picture
