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
 *  recordVideo.cpp
 *
 *  Tool for evaluating fuzzer-produced trajectories. Adapted from
 *  videoRecordingExample.cpp in ALE.
 **************************************************************************** */

#include <iostream>
#include <ale_interface.hpp>
#include <cstdlib>
#include "fuzzableCommon.h"

#ifndef ALE_SDL_SUPPORT
#error Video recording example is disabled as it requires SDL. Recompile with -DSDL_SUPPORT=ON.
#else

using namespace std;

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " rom_file record_path" << std::endl;
    return 1;
  }

  ale::ALEInterface ale;

  // Get & Set the desired settings
  ale.setInt("random_seed", 123);

  // We enable both screen and sound, which we will need for recording.
  ale.setBool("display_screen", true);
  // You may leave sound disabled (by setting this flag to false) if so desired.
  ale.setBool("sound", true);

  std::string recordPath{argv[2]};
  std::cout << std::endl;

  // Set record flags
  ale.setString("record_screen_dir", recordPath.c_str());
  ale.setString("record_sound_filename", (recordPath + "/sound.wav").c_str());
  // We set fragsize to 64 to ensure proper sound sync
  ale.setInt("fragsize", 64);

  // Not completely portable, but will work in most cases
  std::string cmd = "mkdir ";
  cmd += recordPath;
  system(cmd.c_str());

  // Load the ROM file. (Also resets the system for new settings to
  // take effect.)
  ale.loadROM(argv[1]);

  // Play a single episode, which we record.
  CharFeed charFeed;
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

  std::cout << std::endl;
  std::cout << "Recording complete. To create a video, you may want to run "
               "joinVideo.sh."
            << std::endl;

  return 0;
}
#endif  // ALE_SDL_SUPPORT
