devenv bullet-2.79\msvc\vs2015\0BulletSolution.sln /build Debug /project BulletCollision /projectconfig "Debug|Win32"
devenv bullet-2.79\msvc\vs2015\0BulletSolution.sln /build Debug /project BulletDynamics  /projectconfig "Debug|Win32"
devenv bullet-2.79\msvc\vs2015\0BulletSolution.sln /build Debug /project LinearMath      /projectconfig "Debug|Win32"

devenv DXUT11\Core\DXUT_2015.sln           /build Debug /project DXUT      /projectconfig "Debug|Win32"
devenv DXUT11\Optional\DXUTOpt_2015.sln    /build Debug /project DXUTOpt   /projectconfig "Debug|Win32"
devenv DXUT11\Effects11\Effects11_2015.sln /build Debug /project Effects11 /projectconfig "Debug|Win32"

devenv libogg-1.3.0\win32\VS2015\libogg_static.sln /build Debug /project libogg_static /projectconfig "Debug|Win32"

devenv libvorbis-1.3.2\win32\VS2015\vorbis_static.sln /build Debug /project libvorbisfile /projectconfig "Debug|Win32"

devenv luaplus51-all\build2015\luaplus51-1201.sln /build Debug /project luaplus51-1201 /projectconfig "Debug|Win32"

devenv tinyxml_2_6_2\tinyxml.sln /build Debug /project tinyxml /projectconfig "Debug|Win32"

devenv zlib-1.2.5\contrib\vstudio\vc14\zlibvc.sln /build Debug /project zlibstat  /projectconfig "Debug|Win32"

REM copyalllibs_vs2015
