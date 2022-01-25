#ifndef __FUZZABLE_COMMON_H__
#define __FUZZABLE_COMMON_H__

#include <optional>
#include <ale_interface.hpp>

// TODO(sam): refactor implementation into fuzzableCommon.c

std::optional<ale::Action> getAction(char actChar) {
  // Character 'a' corresponds to the first action in the Action enum (in
  // Constants.h). Each of the PLAYER_A_* actions is assigned a subsequent
  // letter in order.
  int actIdx = ((int)(actChar - 'a')) % PLAYER_A_MAX;
  if (actIdx < 0 || actIdx >= PLAYER_A_MAX) {
    return {};
  }
  return (ale::Action)actIdx;
}

class CharFeed {
 public:
  CharFeed() {}

  std::optional<char> nextChar() {
    if (unreadStart >= lastReadSize) {
      // read again
      unreadStart = 0;
      lastReadSize = fread(buffer.data(), 1, capacity, stdin);
      if (lastReadSize == 0) {
        if (feof(stdin)) {
          // signal EOF
          return {};
        } else if (ferror(stdin)) {
          // print error & then treat as EOF
          perror("fread");
          clearerr(stdin);
          return {};
        }
      }
    }
    return buffer[unreadStart++];
  }

 private:
  static const unsigned int capacity = 1024;
  unsigned int unreadStart = 0;
  size_t lastReadSize = 0;
  std::array<char, capacity> buffer;
};

#endif // __FUZZABLE_COMMON_H__
