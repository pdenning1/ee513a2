#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include "publish.h"


using namespace std;
using namespace exploringRPi;
using namespace ee513a2;

int main(int argc, char* argv[]) {

    Publish thePublisher;

    thePublisher.publishAll();

    int count = 0;
    int iterations = 1000000;
    while(count < iterations){
        //sleep(1);
	usleep(100000);
        thePublisher.publishAll();
	count++;
    }

    thePublisher.disconnect();
}
