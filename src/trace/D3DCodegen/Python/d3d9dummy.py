#
# Generates C++ code for a dummy (All interfaces receive calls, but aren't
# implemented) d3d9 driver.
# 
# Initial version - chema (csolis@ac.upc.edu)
#


import d3d9api
from d3d9api import *


api = getD3D9API("d3d9api.xml")

f = open("D3DDriver.h","w")

returnedInterfaces = []
returnedInterfaces.append("IDirect3D9**")
returnedInterfaces.append("IDirect3DDevice9**")
returnedInterfaces.append("IDirect3DSwapChain9**")
returnedInterfaces.append("IDirect3DTexture9**")
returnedInterfaces.append("IDirect3DVolumeTexture9**")
returnedInterfaces.append("IDirect3DCubeTexture9**")
returnedInterfaces.append("IDirect3DVertexBuffer9**")
returnedInterfaces.append("IDirect3DIndexBuffer9**")
returnedInterfaces.append("IDirect3DSurface9**")
returnedInterfaces.append("IDirect3DVolume9**")
returnedInterfaces.append("IDirect3DVertexDeclaration9**")
returnedInterfaces.append("IDirect3DVertexShader9**")
returnedInterfaces.append("IDirect3DPixelShader9**")
returnedInterfaces.append("IDirect3DStateBlock9**")
returnedInterfaces.append("IDirect3DQuery9**")
returnedInterfaces.append("IDirect3DBaseTexture9**")
returnedInterfaces.append("IDirect3DResource9**")

interfacesByName = {}
for i in api.interfaces:
    interfacesByName[i.name] = i



print >>f, """
/**
 * 
 * Autogenerated dummy (calls aren't operative) d3d9 driver.
 *  - Returned interface addresses are valid, using singleton interfaces objects.
 *  - Returned memory addresses are valid, using a common shared buffer.
 *  - A warning is generated in cout in each function called.
 *  
 */
 
#ifndef _D3D_DRIVER
#define _D3D_DRIVER

#include <d3d9_port.h>

"""

# Code to be included after IDirect3DVolume9 implementation class declaration
volumePatchH = """
    virtual DWORD D3D_CALL SetPriority(DWORD);
    virtual DWORD D3D_CALL GetPriority();
    virtual void D3D_CALL PreLoad();
    virtual D3DRESOURCETYPE D3D_CALL GetType();
"""

for i in api.interfaces:
    imp_class = "D3D" + i.name[9:]
    print >>f, "class", imp_class + " : public " + i.name + "{"
    print >>f, "public:"
    print >>f, "    static", imp_class , "&getInstance();"
    for m in i.methods:
        print >>f, "   ", m.returns, "D3D_CALL", m.name, "(",
        comma = ""
        for p in m.parameters:
            print >>f, comma, p.type, p.name,
            comma = ","
        print >>f, ");"
    # IDirect3DVolume9 does not redeclare Resource methods
    if i.name == "IDirect3DVolume9":
        print >>f, volumePatchH
    print >>f, "private:"
    print >>f, "    " + imp_class + "();"
    print >>f, "};"
    print >>f, ""

for ff in api.functions:
    # Some functions have empty return type and ISO C++ forbids this.
    # Void is generated in this case.
    returns = "void"
    if ff.returns != "":
        returns = ff.returns
    print >>f, returns, "D3D_CALL", ff.name, "("
    comma = ""
    for p in ff.parameters:
        print >>f, comma, p.type, p.name
        comma = ","
    print >>f, ");"

print >>f, """
#endif

"""

f.close()


f = open("D3DDriver.cpp","w")

print >>f, """
#include "D3DDriver.h"
#include <iostream>
using namespace std;

char buffer[1024 * 1024]; // 1MB buffer for lock/unlock

"""

# Code to be included after IDirect3DVolume9 implementation class definition
volumePatchCPP = """
DWORD D3DVolume9 :: SetPriority(DWORD) {
    cout << "WARNING: IDirect3DVolume9 :: SetPriority NOT IMPLEMENTED" << endl;
    return static_cast<DWORD>(0);
}

DWORD D3DVolume9 :: GetPriority() {
    cout << "WARNING: IDirect3DVolume9 :: GetPriority NOT IMPLEMENTED" << endl;
    return static_cast<DWORD>(0);
}

void D3DVolume9 :: PreLoad() {
    cout << "WARNING: IDirect3DVolume9 :: PreLoad NOT IMPLEMENTED" << endl;
}

D3DRESOURCETYPE D3DVolume9 :: GetType() {
    cout << "WARNING: IDirect3DVolume9 :: GetType NOT IMPLEMENTED" << endl;
    return static_cast<D3DRESOURCETYPE>(0);
}
"""

# Generate implementation classes
for i in api.interfaces:
    imp_class = "D3D" + i.name[9:]
    print >>f, imp_class + " :: " + imp_class + "() {}"
    print >>f, ""    
    print >>f, imp_class, "&", imp_class + " :: getInstance() {"
    print >>f, "    static",imp_class + " instance;"
    print >>f, "    return instance;"
    print >>f, "}"
    print >>f, ""
    for m in i.methods:
        print >>f, m.returns, "D3D_CALL", imp_class, "::", m.name, "(",
        comma = ""
        for p in m.parameters:
            print >>f, comma, p.type, p.name,
            comma = ","
        print >>f, ") {"
        for p in m.parameters:
            if p.type in returnedInterfaces:
                name = "D3D" + p.type[9:-2]
                print >>f, "    *", p.name, "= &", name, ":: getInstance();"
            elif p.type == "void**":
                print >>f, "    *", p.name, "= buffer;"
            elif p.type == "    D3DLOCKED_RECT*":
                print >>f, "   ", p.name, "->Pitch = 1;"
                print >>f, "   ", p.name, "->pBits = buffer;"
            elif p.type == "D3DLOCKED_BOX*":
                print >>f, "   ", p.name, "->RowPitch = 1;"
                print >>f, "   ", p.name, "->SlicePitch = 1;"
                print >>f, "   ", p.name, "->pBits = buffer;"
        print >> f, "    cout <<\"WARNING: ", i.name, "::", m.name, " NOT IMPLEMENTED\" << endl;" 
        if m.returns != "void":
            print >>f, "   ", m.returns, "ret = static_cast<", m.returns, ">(0);"
            print >>f, "    return ret;"
        print >>f, "}"
        print >>f, ""
    # IDirect3DVolume9 does not redefine Resource methods, so this additional code
    if i.name == "IDirect3DVolume9":
        print >>f, volumePatchCPP
    print >>f, ""        

# Generate global functions
for ff in api.functions:
    # Some functions have empty return type and ISO C++ forbids this.
    # Void is generated in this case.
    returns = "void"
    if ff.returns != "":
        returns = ff.returns
    print >>f, returns, "D3D_CALL", ff.name, "("
    comma = ""
    for p in ff.parameters:
        print >>f, comma, p.type, p.name,
        comma = ","
    print >>f, ") {",
    print >> f, "    cout <<\"WARNING: ", ff.name, " NOT IMPLEMENTED\" << endl;" 
    if (ff.returns != "void") and (ff.returns != ""):
        print >>f, "    return &D3D9::getInstance();"
    print >>f, "}"    

f.close()
