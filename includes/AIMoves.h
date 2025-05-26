#pragma once

#include <gtkmm.h>
#include <array>

#include "Slot.h"

std::array<int, 2> randomAIMove(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots);

