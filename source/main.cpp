#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "annealing.h"
#include "point.h"

std::vector<Point> getInput(std::istream& cin)
{
    std::vector<Point> points;
    Point point;
    unsigned int pos = 0;

    std::string s,t;
    getline(cin,s);

    while ((pos = s.find_first_of(','))!=std::string::npos)
    {
        point.x = stoi(s.substr(0,pos));
        s.erase(0,pos+1);
        if ((pos = s.find_first_of(','))==std::string::npos)
            break;
        point.y = stoi(s.substr(0,pos));
        s.erase(0,pos+1);
        points.push_back(point);
    }

    return points;
}

float** createMatrixOfACompleteGraph(const std::vector<Point>& points)
{
    // memory for matrix
    float** matrix = new float*[points.size()];
    for (unsigned int i = 0; i < points.size(); ++i)
    {
        matrix[i] = new float[points.size()];
        // main diagonal is all zeros
        matrix[i][i] = 0;
    }

    for (unsigned int i = 0; i < points.size(); ++i)
        for (unsigned int j = i+1; j < points.size(); ++j)
            matrix[i][j] = matrix[j][i] =
                distanceBetweenPoints(points[i], points[j]);

    return matrix;
}

void deleteMatrix(float** matrix, int n)
{
    for (int i = 0; i < n; ++i)
        delete[] matrix[i];
    delete[] matrix;
}

int main()
{
    enum {WIDTH = 800, HEIGHT = 600};

    std::cout << "The visualization can be stopped at any time by closing the Finding path window,\n";
    std::cout << "in such case the program will display a currently found permutation.\n\n";
    std::cout << "Enter points x,y, coordinates:\n";
    std::vector<Point> points = getInput(std::cin);

    std::cout << "\nEnter temperature:\n";
    float initialTemperature = 0.0f;
    std::cin >> initialTemperature;
    std::cout << "\nEnter q:\n";
    float q = 0.0f;
    std::cin >> q;

    float** matrix = createMatrixOfACompleteGraph(points);

    int sleepRestriction = 0;
    std::cout << "\nSlow down the visualization on initial steps?\n0 - No\nOther - Yes\n";
    std::cin >> sleepRestriction;
    std::cin.ignore();
    if (sleepRestriction != 0)
        sleepRestriction = 1;

    // annealing initial state
    Path currentPath = initializeNewPath(matrix, points.size());

    int withoutChangeCounter = 0;
    // after how many temperature changes without changing path the algorithm will stop
    int threshold = 5000;
    float temperature = initialTemperature;
    bool annealing = true;

    // start visualization
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Finding path");

    // Create background.
    sf::RenderTexture background;
    if (!background.create(WIDTH, HEIGHT))
        return -1;
    background.clear(sf::Color::Black);

    // connection between vertices
    sf::Vertex line[2];

    // radius of vertices
    const float radius = 5.0f;
    // draw each vertex on background
    for (unsigned int i = 0; i < points.size(); ++i)
    {
        sf::CircleShape vertex(radius);
        sf::Vector2f middle(float(points[i].x), float(points[i].y));

        vertex.setPosition(middle-sf::Vector2f(radius, radius));
        vertex.setFillColor(sf::Color::White);
        // Draw vertex on the background.
        background.draw(vertex);
    }
    sf::Sprite backgroundVertices(background.getTexture());
    backgroundVertices.setTextureRect(sf::IntRect(0, HEIGHT, WIDTH, -HEIGHT));

    // The main loop.
    while (window.isOpen())
    {
        window.clear(sf::Color::Black);
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // annealing
        annealing = (withoutChangeCounter < threshold) &&
                    (temperature > 0.001f);

        if (annealing)
        {
            if (getNewPath(currentPath, matrix, temperature))
            {
                withoutChangeCounter = 0;
                sf::sleep(sf::milliseconds(sleepRestriction*temperature*40));
            }
            else
                ++withoutChangeCounter;

            // lower the temperature
            temperature*= q;
        }
        else
        {
            std::cout << "######################" << std::endl;
            std::cout << "PERMUTATION FOUND:" << std::endl;
            printPath(currentPath);
            std::cout << "Press enter to exit." << std::endl;
            std::cin.get();
            window.close();
        }

        // Actual drawing.

        // draw vertices
        window.draw(backgroundVertices);

        // draw connections between vertices
        for (int i = 0; i < currentPath.sizePath-1; ++i)
        {
            int v1 = currentPath.path[i];
            int v2 = currentPath.path[i+1];
            sf::Vector2f start(float(points[v1].x), float(points[v1].y));
            sf::Vector2f end(float(points[v2].x), float(points[v2].y));

            line[0].position = start;
            line[1].position = end;
            //draw the line
            window.draw(line, 2, sf::Lines);
        }
        // and between first and last
        int v1 = currentPath.path[0];
        int v2 = currentPath.path[currentPath.sizePath-1];
        sf::Vector2f start(float(points[v1].x), float(points[v1].y));
        sf::Vector2f end(float(points[v2].x), float(points[v2].y));

        line[0].position = start;
        line[1].position = end;
        // draw the line
        window.draw(line, 2, sf::Lines);

        window.display();
    }
    // end of visualization




    if (annealing)
    {
        std::cout << "###############################" << std::endl;
        std::cout << "VISUALIZATION STOPPED MANUALLY" << std::endl;
        std::cout << "PERMUTATION FOUND:" << std::endl;
        printPath(currentPath);
        std::cout << "Press enter to exit." << std::endl;
        std::cin.get();
    }

    deleteMatrix(matrix, points.size());
    deletePath(currentPath);
    return 0;
}
