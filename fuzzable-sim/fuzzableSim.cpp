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

#include <iostream>
#include <ale_interface.hpp>

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
  ale.setBool("display_screen", true);
  ale.setBool("sound", true);
#endif

  // Load the ROM file. (Also resets the system for new settings to
  // take effect.)
  ale.loadROM(argv[1]);

  // Get the vector of legal actions
  ale::ActionVect legal_actions = ale.getLegalActionSet();

  float totalReward = 0;
  int steps = 0;
  while (!ale.game_over()) {
    char actChar;
    std::cin >> actChar;
    // Character 'a' corresponds to the first action in the Action enum (in
    // Constants.h). Each of the PLAYER_A_* actions is assigned a subsequent
    // letter in order.
    int actIdx = ((int)(actChar - 'a')) % PLAYER_A_MAX;
    if (actIdx < 0 || actIdx >= PLAYER_A_MAX) {
      continue;
    }
    ale::Action a = (ale::Action)actIdx;
    totalReward += ale.act(a);
    steps++;
  }
  std::cout << "Steps:" << steps << std::endl;
  std::cout << "Reward: " << totalReward << std::endl;

  return 0;
}
