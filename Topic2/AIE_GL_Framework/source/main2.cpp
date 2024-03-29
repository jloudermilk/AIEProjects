//////////////////////////////////////////////////////////////////////////
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/glfw.h>

#include "Utilities.h"
#include "Visualiser.h"
#include "Topic2.h"



using namespace AIE;
//////////////////////////////////////////////////////////////////////////
void Init();
void Shutdown();
void Update();
void Draw();

GLuint g_ShaderID = 0;
GLuint g_Texture1ID = 0;
GLuint g_Texture2ID = 0;

GLuint g_VAO = 0;
GLuint g_VBO = 0;
GLuint g_IBO = 0;

mat4 g_ProjectionMatrix;
mat4 g_ViewMatrix;
mat4 g_ModelMatrix;

mat4 g_CameraMatrix;

float g_fPhase = 0.0f;
//////////////////////////////////////////////////////////////////////////
int main()
{
	// start GLFW and open a window
	glfwInit();

	// get the desktop mode
	GLFWvidmode oMode;
	glfwGetDesktopMode(&oMode);

	// open a window
	glfwOpenWindow(	1280, 720,	// resolution
					oMode.RedBits,oMode.GreenBits,oMode.BlueBits,0,	// bits per colour channel (RGBA)
					24,			// depth bits
					8,			// stencil bits
					GLFW_WINDOW);
	glfwSetWindowTitle("AIE OpenGL Window");

	// start GLEW
	if (glewInit() != GLEW_OK)
	{
		// OpenGL didn't start-up! shutdown GLFW and return an error code
		glfwTerminate();
		return -1;
	}

	// initialise our own stuff
	Init();

	do 
	{
		// update and draw our stuff to the back-buffer
		Update();		
		Draw();

		// bring the back-buffer to the monitor
		glfwSwapBuffers();

	} while ( glfwGetKey( GLFW_KEY_ESC ) == GLFW_RELEASE &&
			  glfwGetWindowParam( GLFW_OPENED ) != 0 );
	
	// clean up our stuff
	Shutdown();

	// shutdown GLFW
	glfwTerminate();

	// return success
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void Init()
{


	// setup matrices
	g_CameraMatrix.SetFrame( vec4(0,10,-10,1), vec4(0,-1,1,0), vec4(0,1,0,0));
	g_ProjectionMatrix.Perspective(PI/6, 1200/720.0f, 0.1f, 100);
//	g_ProjectionMatrix.Orthographic(1280,720,0.1f,100);
	g_ViewMatrix = g_CameraMatrix.ToViewMatrix();
	g_ModelMatrix = mat4(1,0,0,0,
						0,1,0,0,
						0,0,1,0,
						0,0,0,1);
	// load shader
	const char* aszInputs[] = { "Position",	"UV" };
	const char* aszOutputs[] = { "outColor" };
	g_ShaderID = LoadShader( 2, aszInputs, 1, aszOutputs,
		"./shaders/vertex.glsl",
		"./shaders/fragment.glsl");

	// build 2-triangle plane
	float fPlaneSize = 2.0f;
	Build3DPlane(fPlaneSize,g_VAO,g_VBO,g_IBO);

	// load texture
	g_Texture1ID = LoadTexture("./images/earth_diffuse.tga", GL_BGRA);
	g_Texture2ID = LoadTexture("./images/earth_lights.tga", GL_BGRA);

	// set matrix uniforms within the shaders
	GLuint ProjectionID = glGetUniformLocation(g_ShaderID,"Projection");
	GLuint ViewID = glGetUniformLocation(g_ShaderID,"View");
	GLuint ModelID = glGetUniformLocation(g_ShaderID,"Model");

	glUniformMatrix4fv(ProjectionID, 1, false, g_ProjectionMatrix);
	glUniformMatrix4fv(ViewID, 1, false, g_ViewMatrix);
	glUniformMatrix4fv(ModelID, 1, false, g_ModelMatrix);

	GLuint PhaseID = glGetUniformLocation(g_ShaderID,"Phase");


	
	// set clear colour
	glClearColor(0.25f,0.25f,0.25f,1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// start our timer
	AIE::ResetTimer();
}

//////////////////////////////////////////////////////////////////////////
void Update()
{
	float fDeltaTime = GetDeltaTime();

	// update movement of the camera's world transform
	FreeMovement(fDeltaTime,g_CameraMatrix,10);

	// example rotated world transform for the crate plane
	static float sfTimer = 0;
	sfTimer += fDeltaTime;
	
	g_fPhase += fDeltaTime;
	if(g_fPhase > PI){
		g_fPhase = 0;
	}


}

//////////////////////////////////////////////////////////////////////////
void Draw()
{
	// clear the backbuffer to our clear colour and clear the depth
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set active shader
	glUseProgram(g_ShaderID);
	
	GLuint texUniform1ID = glGetUniformLocation(g_ShaderID,"diffuseTexture");
	glUniform1i(texUniform1ID,0);

	GLuint texUniform2ID = glGetUniformLocation(g_ShaderID,"lightTexture");
	glUniform1i(texUniform2ID,1);


	// convert camera's world matrix to a view matrix
	g_ViewMatrix = g_CameraMatrix.ToViewMatrix();

	// set current transforms in the shader
	GLuint ViewID = glGetUniformLocation(g_ShaderID,"View");
	glUniformMatrix4fv(ViewID, 1, false, g_ViewMatrix);

	GLuint ModelID = glGetUniformLocation(g_ShaderID,"Model");
	glUniformMatrix4fv(ModelID, 1, false, g_ModelMatrix);

	GLuint PhaseID = glGetUniformLocation(g_ShaderID,"Phase");
	glUniform1f(PhaseID,g_fPhase );

	// set active texture, bind the crate quad's buffers and draw it
		glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_Texture1ID );

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_Texture2ID );

	glBindVertexArray(g_VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);




}

//////////////////////////////////////////////////////////////////////////
void Shutdown()
{
	// destroy buffers
	Visualiser::Destroy();
	glDeleteTextures(1,&g_Texture1ID);
	glDeleteTextures(1,&g_Texture2ID);
	glDeleteVertexArrays(1, &g_VAO);
	glDeleteBuffers(1, &g_VBO);
	glDeleteBuffers(1, &g_IBO);
	glDeleteShader(g_ShaderID);
}