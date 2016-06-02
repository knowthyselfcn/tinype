
//#define _AFXDLL
#include "definition.h"
#include "tool.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vulkan/vulkan.h"

#ifdef _WIN32
#include <afxwin.h>
#endif

#include <vector>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#define ENABLE_VALIDATION false


using namespace std;


void test()
{
    for (int i = 0; i < 10000; i++){
        cout << drand48() << endl;
        Sleep(61);
    }

    ParticleForceRegistry::ParticleForceRegistration reg;

}

 





//int main(void)
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{

    //HINSTANCE hInstance = AfxGetInstanceHandle();
     

     
    system("pause");
    return 0;
}