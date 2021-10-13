//
// Created by root on 10/12/21.
//

#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>

using namespace std;

static int pipe_fd[2];

void sig_handle(int signo){
    int save_errno = errno;
    cout << "capture signal " << signo << endl;
    write(pipe_fd[0], &signo, sizeof(signo));
    errno = save_errno;
}

int main(){
    do{
        if(socketpair(AF_UNIX, SOCK_STREAM, 0, pipe_fd) < 0){
            cout << strerror(errno) << endl;
            break;
        }

        struct sigaction act;
        sigfillset(&act.sa_mask);
        act.sa_handler = sig_handle;
        act.sa_flags |= SA_RESTART;
        if(sigaction(SIGINT, &act, nullptr) < 0){
            cout << strerror(errno) << endl;
            break;
        }

        int signo;
        while (true){
            if (read(pipe_fd[1], &signo, sizeof(signo)) > 0){
                cout << "receive signal " << signo << endl;
                break;
            }
        }
        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }while(false);
    return 0;
}