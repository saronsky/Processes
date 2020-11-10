/*
Simon Aronsky
CSS430
Project 1C
10/19/2020
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

void forkError();

/*
 * This function uses grep, ps, and wc to count the number of proccesses
 * containing name argv[1]
 *
 * PreConditions
 *   argv:  A String array of search keyword
 *   argc:  The number of strings in the string array
 *
 */
int main(int argc, char *argv[]) {
  if (argc < 2) {
    perror("Too Few Arguments ERROR");
    exit(EXIT_FAILURE);
  }
  if (argc > 2) {
    perror("Too Many Arguments ERROR");
    exit(EXIT_FAILURE);
  }

  int status;
  enum { RD, WR };
  int pipeFD1[2]; // pipe between PS and Grep
  int pipeFD2[2]; // pipe between Grep and WC
  pid_t pid;

  if (pipe(pipeFD1) < 0) {
    perror("Pipe 1 ERROR");
    exit(EXIT_FAILURE);
  }
  if (pipe(pipeFD2) < 0) {
    perror("Pipe 2 ERROR");
    exit(EXIT_FAILURE);
  }

  if ((pid = fork()) < 0)
    forkError();
  else if (pid == 0) { // Local Child Global Child Process (WC)
    if ((pid = fork()) < 0)
      forkError();
    else if (pid == 0) { // Local Child Global GrandChild Process (Grep)
      if ((pid = fork()) < 0)
        forkError();
      else if (pid == 0) { // Local Child Global GreatGrandChild Process (PS)
        close(pipeFD2[RD]);
        close(pipeFD2[WR]);
        close(pipeFD1[RD]);
        dup2(pipeFD1[WR], WR);
        if (execlp("ps", "ps", "-A", (char *)0) == -1) {
          perror("PS Execution ERROR");
          exit(EXIT_FAILURE);
        }
      } else { // Local Parent Global GrandChild Process (Grep)
        close(pipeFD1[WR]);
        close(pipeFD2[RD]);
        dup2(pipeFD1[RD], RD);
        dup2(pipeFD2[WR], WR);
        if (execlp("grep", "grep", argv[1], (char *)0) == -1) {
          perror("Grep Execution ERROR");
          exit(EXIT_FAILURE);
        }
      }
    } else { // Local Parent Global Child Process (WC)
      close(pipeFD1[WR]);
      close(pipeFD1[RD]);
      close(pipeFD2[WR]);
      dup2(pipeFD2[RD], RD);
      if (execlp("wc", "wc", "-l", (char *)0) == -1) {
        perror("WC Execution ERROR");
        exit(EXIT_FAILURE);
      }
    }
  } else { // Local Parent Global Parent Process (BASH)
    // CLOSE ALL PIPES
    close(pipeFD1[RD]);
    close(pipeFD1[WR]);
    close(pipeFD2[RD]);
    close(pipeFD2[WR]);
    wait(&status);
  }
  return 0;
}

/*
 * This function outputs an Error string and exits the program if a
 * Fork Error occurs
 */
void forkError() {
  perror("Fork ERROR");
  exit(EXIT_FAILURE);
}
