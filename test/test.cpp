
#include "definition.h"
#include "tool.h"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>


using namespace std;

int main(void)
{



    for (int i = 0; i < 10000; i++){
        cout << drand48() << endl;
        Sleep(61);
    }

    ParticleForceRegistry::ParticleForceRegistration reg;

     
    system("pause");
    return 0;
}