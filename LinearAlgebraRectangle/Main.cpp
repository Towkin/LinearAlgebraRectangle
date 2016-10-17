#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include "Entity.h"
#include "SFML/Graphics.hpp"
#include <map>

// a
static int LastDigits = 3;

int main() {
	
	std::ifstream LoadStream;
	LoadStream.open("settings.ini");
	if (LoadStream.is_open()) {
		std::string Line;
		while (std::getline(LoadStream, Line)) {
			size_t Value = Line.find("=");
			if (Value != std::string::npos) {
				
				LastDigits = std::stoi(Line.substr(Value + 1));
			}
		}
		LoadStream.close();
	}
	

	// Setting up the variables. Note: some values have an inverted y-axis.
	Rotation StartAngle(-(5.f * LastDigits + 30), ERotation::Degrees);
	float StartSpeed = LastDigits;
	Eigen::Vector2f StartVelocity = StartAngle * Eigen::Vector2f(StartSpeed, 0.f);
	float AngularVelocity = 100.f * LastDigits;
	float GravityAcceleration = 9.82f;

	Eigen::Vector2f StartPosition(0.f, 1.f + 0.1f * LastDigits);
	Eigen::Vector2f ConstantAcceleration(0.f, -GravityAcceleration);

	float RectangleWidth = 0.1f * LastDigits;
	float RectangleHeight = 0.05f * LastDigits;

	sf::RenderWindow MainWindow(sf::VideoMode(800, 600), "Linear Algebra - Rotating Rectangle");

	// Setup the entities.
	Entity CenterOfMass;
	Entity Verticies[4];
	
	// Set the COM at the start position.
	CenterOfMass.SetPosition(StartPosition, EContext::Global);

	Eigen::Vector2f COMOffset = Eigen::Vector2f(RectangleWidth * -0.1f * LastDigits, RectangleHeight * -0.1f * LastDigits);
	Eigen::Vector2f RectangleCenter = StartPosition - COMOffset - Eigen::Vector2f(RectangleWidth / 2, RectangleHeight / 2);

	// Setup the verticies, using global coordinates.
	for (int i = 0; i < 4; i++) {
		// First attach the rectangles to the center of mass.
		Verticies[i].AttachTo(&CenterOfMass);

		// Go through the corners in TopLeft, TopRight, BottomLeft, BottomRight order.
		float MultX = (i % 2) - 0.5f;
		float MultY = (i / 2) - 0.5f;

		// When position is set in the Global context, relative position is modified
		// Note, the MultX and Y also deals with the half-width and -height.
		Verticies[i].SetPosition(RectangleCenter + Eigen::Vector2f(MultX * RectangleWidth, MultY * RectangleHeight), EContext::Global);
	}

	sf::Color COMColor(192, 192, 192);
	sf::Color CornerColors[4] { sf::Color::Blue, sf::Color::Green, sf::Color::Magenta, sf::Color::Red };

	sf::CircleShape COMPoint(0.01f, 8.f);
	COMPoint.setFillColor(COMColor);

	sf::VertexArray RectVerticies(sf::PrimitiveType::TrianglesStrip, 4);
	for (int i = 0; i < 4; i++) {
		RectVerticies[i].color = CornerColors[i];
	}
	
	float V0_y = -StartVelocity[1];
	float G_y = GravityAcceleration;
	float P0_y = StartPosition[1];

	float Time = 0.f;
	float TimeScale = 1.f;
	float LastTimeScale = TimeScale;
	float TimeMax = 
		(V0_y / G_y) * (
			1.f + sqrtf(
				1.f + (
					2.f * P0_y * G_y / powf(
						V0_y, 2.f
					)
				)
			)
		);

	float LineRenderDelta = 0.005f;

	sf::Clock Timer;
	sf::Time MinFrameTime = sf::seconds(1.f / 60.f);
	
	sf::View MainView;
	MainView.setSize(sf::Vector2f(1.f, 0.75f) * 2.f);
	MainView.setCenter(sf::Vector2f(RectangleCenter[0], -RectangleCenter[1]));

	sf::Vector2f LastMousePos;
	bool MousePressed = false;
	std::map<sf::Keyboard::Key, bool> LastKeyState;
	LastKeyState[sf::Keyboard::Space] = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
	LastKeyState[sf::Keyboard::BackSpace] = sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace);

	while (MainWindow.isOpen()) {
		// Save new mouse position to get delta later.
		sf::Vector2f NewMousePos = (sf::Vector2f)sf::Mouse::getPosition();

		if (!MainWindow.hasFocus()) {
			sf::sleep(MinFrameTime);
			continue;
		}
		
		int MouseWheelDelta = 0;
		
		sf::Event WindowEvent;
		while (MainWindow.pollEvent(WindowEvent)) {
			if (WindowEvent.type == sf::Event::Closed) {
				MainWindow.close();
			}
			if (WindowEvent.type == sf::Event::MouseWheelMoved) {
				MouseWheelDelta = WindowEvent.mouseWheel.delta;
			}
		}
		
		
		// --- Time ---

		// On pressed Space, pause or resume time.
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && !LastKeyState[sf::Keyboard::Space]) {
			TimeScale = TimeScale == 0.f ? LastTimeScale : 0.f;
		}
		LastKeyState[sf::Keyboard::Space] = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

		// On pressed Backspace, reset time and pause.
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace) && !LastKeyState[sf::Keyboard::BackSpace]) {
			Time = 0.f;
			TimeScale = 0.f;
		}
		LastKeyState[sf::Keyboard::BackSpace] = sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace);

		// Increase or decrease time speed.
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
			TimeScale -= Timer.getElapsedTime().asSeconds();
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			TimeScale += Timer.getElapsedTime().asSeconds();
		}

		// Play or Reverse.
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			TimeScale = 1.f;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			TimeScale = -1.f;
		}

		// Acuumulate time from timer.
		Time += TimeScale * Timer.restart().asSeconds();
		// Clamp the time.
		Time = std::fmaxf(std::fminf(Time, TimeMax), 0.f);

		// Pause if on end or begining of simulation.
		if (Time == 0.f || Time == TimeMax) {
			TimeScale = 0.f;
		}

		if (TimeScale != 0.f) {
			LastTimeScale = TimeScale;
		}


		// --- View ---

		// If left mouse button is held.
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			
			sf::Vector2f MouseDelta = LastMousePos - NewMousePos;
			// Scale the delta according to window size.
			sf::Vector2f MoveDelta(
				(MouseDelta.x / MainWindow.getSize().x) * MainView.getSize().x, 
				(MouseDelta.y / MainWindow.getSize().y) * MainView.getSize().y
			);

			// Move the view.
			MainView.setCenter(MainView.getCenter() +  MoveDelta);
		}
		LastMousePos = NewMousePos;

		// View zooming through exponential function. 0.95 is a good value.
		MainView.setSize(MainView.getSize() * powf(0.95f, MouseWheelDelta));
		MainWindow.setView(MainView);


		// --- Rectangle Verticies ---

		// Set the rotation and position according to simulation.
		CenterOfMass.SetRotation(Rotation(AngularVelocity * Time, ERotation::Degrees));
		CenterOfMass.SetPosition(-StartPosition + StartVelocity * Time - ((ConstantAcceleration * powf(Time, 2.f)) / 2.f), EContext::Global);

		// Save the vector to do a Eigen->SFML conversion, then set the position of the rendered CenterOfMass circle.
		Eigen::Vector2f COMPos = CenterOfMass.GetPosition(EContext::Global);
		COMPoint.setPosition(COMPos[0] - COMPoint.getRadius(), COMPos[1] - COMPoint.getRadius());

		// Go through the rectangle's verticies and update positions. Again, Eigen::Vector2f()->sf::Vector2f() conversion.
		for (int i = 0; i < 4; i++) {
			Eigen::Vector2f VertexPos = Verticies[i].GetPosition(EContext::Global);
			RectVerticies[i].position.x = VertexPos[0];
			RectVerticies[i].position.y = VertexPos[1];
		}

		
		// --- Grid ---

		// Setup an empty grid in loop-scope.
		sf::VertexArray Grid;

		// Gridsize is at least 0.1, and may be any power of 10 above it, based on zoom level.
		float GridSize = 0.01f;
		while (GridSize < 20.f * MainView.getSize().x / MainWindow.getSize().x) {
			GridSize *= 10.f;
		}
		
		if (GridSize > 0.f) {
			// Get the current top left point of the view.
			float ViewLeft = MainView.getCenter().x - MainView.getSize().x / 2;
			float ViewTop = MainView.getCenter().y - MainView.getSize().y / 2;

			// Where the grid should start in the current view.
			float GridStartX = ViewLeft - std::fmodf(ViewLeft, GridSize);
			float GridStartY = ViewTop - std::fmodf(ViewTop, GridSize);

			// How many lines are within the current view.
			int LineCountX = 2 + (int)(MainView.getSize().x / GridSize);
			int LineCountY = 2 + (int)(MainView.getSize().y / GridSize);

			// The vertex arrays needs twice the amount of verticies to make the lines.
			Grid = sf::VertexArray(sf::PrimitiveType::Lines, LineCountX * 2 + LineCountY * 2);

			for (int i = 0; i < Grid.getVertexCount(); i++) {
				Grid[i].color = sf::Color(64, 64, 64);
			}

			// Set the vertex positions to draw the grid.
			for (int i = 0; i < LineCountX; i++) {
				int x0 = i * 2;
				int x1 = x0 + 1;

				float GridX = GridStartX + i * GridSize;

				Grid[x0].position = sf::Vector2f(GridX, ViewTop);
				Grid[x1].position = sf::Vector2f(GridX, ViewTop + MainView.getSize().y);
			}
			for (int i = 0; i < LineCountY; i++) {
				int y0 = LineCountX * 2 + i * 2;
				int y1 = y0 + 1;

				float GridY = GridStartY + i * GridSize;

				Grid[y0].position = sf::Vector2f(ViewLeft,						  GridY);
				Grid[y1].position = sf::Vector2f(ViewLeft + MainView.getSize().x, GridY);
			}
		}


		// --- Trajectory Lines ---

		// Number of steps needed to draw the entire thing.
		int TimeSteps = Time / LineRenderDelta + 2;

		sf::VertexArray COMLine(sf::PrimitiveType::LinesStrip, TimeSteps);
		sf::VertexArray CornerLines[4];

		for (int i = 0; i < 4; i++) {
			CornerLines[i] = sf::VertexArray(sf::PrimitiveType::Lines, TimeSteps);
		}

		for (int t = 0; t < TimeSteps; t++) {

			float TimeT = std::fminf(t * LineRenderDelta, Time);

			CenterOfMass.SetRotation(Rotation(AngularVelocity * TimeT, ERotation::Degrees));
			CenterOfMass.SetPosition(-StartPosition + StartVelocity * TimeT - ((ConstantAcceleration * powf(TimeT, 2.f)) / 2.f), EContext::Global);

			Eigen::Vector2f DotPos = CenterOfMass.GetPosition(EContext::Global);

			COMLine[t].position.x = DotPos[0];
			COMLine[t].position.y = DotPos[1];
			COMLine[t].color = sf::Color(192, 192, 192);

			for (int i = 0; i < 4; i++) {
				Eigen::Vector2f VertexPos = Verticies[i].GetPosition(EContext::Global);
				
				CornerLines[i][t].position.x = VertexPos[0];
				CornerLines[i][t].position.y = VertexPos[1];
				CornerLines[i][t].color = CornerColors[i];
			}
		}


		// --- Window drawing ---

		MainWindow.clear();

		MainWindow.draw(Grid);
		
		MainWindow.draw(RectVerticies);
		MainWindow.draw(COMPoint);
		
		for (int i = 0; i < 4; i++) {
			MainWindow.draw(CornerLines[i]);
		}
		MainWindow.draw(COMLine);

		MainWindow.display();

		sf::sleep(MinFrameTime - Timer.getElapsedTime());


		// Modified aspect ratio

		float WindowAspect = ((sf::Vector2f)MainWindow.getSize()).x / ((sf::Vector2f)MainWindow.getSize()).y;
		float ViewAspect = MainView.getSize().x / MainView.getSize().y;

		if (std::abs(WindowAspect - ViewAspect) > 0.001f) {
			MainView.setSize(MainView.getSize().y * WindowAspect, MainView.getSize().y);

			std::cout << "Modified view size: old aspect: " << ViewAspect << ", new aspect: " << MainView.getSize().x / MainView.getSize().y << "\n\n";
		}
	}

	return 0;
}


