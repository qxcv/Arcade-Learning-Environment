/* *****************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2013 by Yavar Naddaf, Joel Veness, Marc G. Bellemare,
 *  Matthew Hausknecht, and the Reinforcement Learning and Artificial Intelligence
 *  Laboratory
 * Released under the GNU General Public License; see License.txt for details.
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  fuzableSim.cpp
 *
 *  Simulator with afl-fuzz adapter. Adapted from examples/cpp-interface/
 *  sharedLibraryInterfaceExample.cpp.
 **************************************************************************** */

#include <cstdio>
#include <iostream>
#include <optional>

#include <ale_interface.hpp>
#include "fuzzableCommon.h"

using namespace std;

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " rom_file" << std::endl;
    return 1;
  }

  ale::ALEInterface ale;

  // Make it deterministic
  ale.setInt("random_seed", 123);
  ale.setFloat("repeat_action_probability", 0.0);

#ifdef ALE_SDL_SUPPORT
  // Don't display anything, want this to be fast under fuzz.
  ale.setBool("display_screen", false);
  ale.setBool("sound", false);
#endif

  // Load the ROM file. (Also resets the system for new settings to
  // take effect.)
  ale.loadROM(argv[1]);

  bool isChild = ale.launchAFLForkServer();
  CharFeed charFeed;
  if (isChild) {
    float totalReward = 0;
    int steps = 0;
    while (!ale.game_over()) {
      auto maybeActChar = charFeed.nextChar();
      if (!maybeActChar) {
        // EOF or read error, stop
        break;
      }
      auto maybeAction = getAction(*maybeActChar);
      if (!maybeAction) {
        // invalid character, stop
        break;
      }
      ale::Action a = *maybeAction;
      totalReward += ale.act(a);
      steps++;
    }
    std::cout << "Steps: " << steps << std::endl;
    std::cout << "Reward: " << totalReward << std::endl;
  }

  return 0;
}
