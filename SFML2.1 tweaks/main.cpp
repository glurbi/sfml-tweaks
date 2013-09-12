#include <iostream>
#include <SFML/Graphics.hpp>
#include <GL/glew.h>

int main()
{
	const int POSITION_ATTRIBUTE_INDEX = 0;

	const int width = 800;
	const int height = 600;
    const float aspectRatio = 1.0f * width / height;

	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 4;
	settings.majorVersion = 3;
	settings.minorVersion = 0;

	sf::RenderWindow window(sf::VideoMode(width, height), "SFML works!", sf::Style::Default, settings);
	glewInit();

	settings = window.getSettings();
	std::cout << "depth bits:" << settings.depthBits << std::endl;
	std::cout << "stencil bits:" << settings.stencilBits << std::endl;
	std::cout << "antialiasing level:" << settings.antialiasingLevel << std::endl;
	std::cout << "version:" << settings.majorVersion << "." << settings.minorVersion << std::endl;

	sf::Font font;
	if (!font.loadFromFile("arial.ttf")) {
		std::cout << "Cannot load arial.ttf" << std::endl;
	}

	window.pushGLStates();

	//
	// create shader program
	//

	// compile vertex shader source
    const char* vertexShaderSource =
		"#version 330\n\
		 uniform mat4 mvpMatrix;\
         uniform vec4 color;\
         in vec3 vpos;\
         out vec4 vcolor;\
		 void main(void) {\
			gl_Position = mvpMatrix * vec4(vpos, 1.0f);\
			vcolor = color;\
		}";
    int vertexShaderSourceLength = strlen(vertexShaderSource);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShaderSource, &vertexShaderSourceLength);
    glCompileShader(vertexShaderId);

	// compile fragment shader source
    const GLchar* fragmentShaderSource = 
		"#version 330\n\
		 in vec4 vcolor;\
         out vec4 fcolor;\
		 void main(void) {\
			fcolor = vcolor;\
		 }";
    int fragmentShaderSourceLength = strlen(fragmentShaderSource);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, &fragmentShaderSource, &fragmentShaderSourceLength);
    glCompileShader(fragmentShaderId);

	// link shader program
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    // associates the "inPosition" variable from the vertex shader with the position attribute
    // the variable and the attribute must be bound before the program is linked
    glBindAttribLocation(programId, POSITION_ATTRIBUTE_INDEX, "position");
    glLinkProgram(programId);

	//
	// create the triangle vertex buffer
	//
	GLuint trianglesId;
    float triangleVertices[] = {
            -0.5f, -0.5f, 0.0f,
            1.0f, -0.5f, 0.0f,
            -0.5f, 1.0f, 0.0f
    };
    glGenBuffers(1, &trianglesId);
    glBindBuffer(GL_ARRAY_BUFFER, trianglesId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

	//
	// create the quad vertex buffer
	//
	GLuint quadId;
    float quadVertices[] = {
            0.5f, 0.5f, 0.0f,
            -1.0f, 0.5f, 0.0f,
            0.5f, -1.0f, 0.0f,
            0.5f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            -1.0f, 0.5f, 0.0f
    };
    glGenBuffers(1, &quadId);
    glBindBuffer(GL_ARRAY_BUFFER, quadId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	//
	// defines the orthographic projection matrix
	//
	float m[16];
	const float left = -1.5f;
	const float right = 1.5f;
	const float bottom = -1.5f / aspectRatio;
	const float top = 1.5f / aspectRatio;
	const float nearPlane = 1.0f;
	const float farPlane = -1.0f;
	m[0] = 2 / (right - left);
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[5] = 2 / (top - bottom);
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = 2 / (farPlane - nearPlane);
    m[11] = 0.0f;
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    m[15] = 1.0f;

	window.popGLStates();

	bool running = true;
    while (running)
    {
		sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
				running = false;
        }

		window.resetGLStates();

        window.clear();
		sf::CircleShape shape(200.f);
		shape.setFillColor(sf::Color::Green);
        window.draw(shape);

		window.pushGLStates();

		//
		// rendering
		//

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		//glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(programId);

		GLuint matrixUniform = glGetUniformLocation(programId, "mvpMatrix");
		glUniformMatrix4fv(matrixUniform, 1, false, m);

		// we need the location of the uniform in order to set its value
		GLuint color = glGetUniformLocation(programId, "color");

		// render the triangle in yellow
		glUniform4f(color, 1.0f, 1.0f, 0.0f, 0.7f);
		glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
		glBindBuffer(GL_ARRAY_BUFFER, trianglesId);
		glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);

		// render the quad in blue
		glUniform4f(color, 0.2f, 0.2f, 1.0f, 0.7f);
		glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
		glBindBuffer(GL_ARRAY_BUFFER, quadId);
		glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);

		window.popGLStates();

		window.resetGLStates();

		sf::Text text;
		text.setFont(font);
		text.setString("Hello world");
		text.setCharacterSize(24);
		text.setColor(sf::Color::Red);
		text.setStyle(sf::Text::Bold | sf::Text::Underlined);
		window.draw(text);
		window.display();

    }

    return 0;
}