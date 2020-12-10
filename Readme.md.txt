This is parts of a project.
If you need to build and test this project, you need make ConsoleApp, install
from Nuget glfw, glm, import Libraries from ./Libraries:
1. glew
	a) includes - .\Libraries\glew-2.1.0\include
	b) libs - .\Libraries\glew-2.1.0\lib\Release\Win32
	c) static link - glew32s.lib
2. SOIL
	a) includes - .\Libraries\simple-opengl-image-library-master\include
	b) libs - .\Libraries\simple-opengl-image-library-master\libs
	c) static link - SOIL.lib
3. Another static linl - opengl32.lib