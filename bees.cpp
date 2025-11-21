//
//  main.cpp
//  CS31 Project 7
//
//  Created by Sienna Curry on 12/5/24.
//

// bees.cpp

// Portions you are to complete are marked with a TODO: comment.
// We've provided some incorrect return statements (so indicated) just
// to allow this skeleton program to compile and run, albeit incorrectly.
// The first thing you probably want to do is implement the trivial
// functions (marked TRIVIAL).  Then get Room::display going.  That gives
// you more flexibility in the order you tackle the rest of the functionality.
// As you finish implementing each TODO: item, remove its TODO: comment.

#include <iostream>
#include <string>
#include <random>
#include <utility>
#include <cstdlib>
using namespace std;

///////////////////////////////////////////////////////////////////////////
// Manifest constants
///////////////////////////////////////////////////////////////////////////

const int MAXROWS = 20;             // max number of rows in the room
const int MAXCOLS = 25;             // max number of columns in the room
const int MAXBEES = 150;            // max number of bees allowed
const int INITIAL_BEE_HEALTH = 2;

const int UP      = 0;
const int DOWN    = 1;
const int LEFT    = 2;
const int RIGHT   = 3;
const int NUMDIRS = 4;

///////////////////////////////////////////////////////////////////////////
// Type definitions
///////////////////////////////////////////////////////////////////////////

class Room;  // This is needed to let the compiler know that Room is a
             // type name, since it's mentioned in the Bee declaration.

class Bee
{
  public:
        // Constructor
    Bee(Room* rp, int r, int c);

        // Accessors
    int  row() const;
    int  col() const;

        // Mutators
    void move();
    bool getSwatted(int dir);

  private:
    Room* m_room;
    int   m_row;
    int   m_col;
    int   m_health;  // Added to track bee health
};

class Player
{
  public:
        // Constructor
    Player(Room *rp, int r, int c);

        // Accessors
    int  row() const;
    int  col() const;
    int  age() const;
    bool isDead() const;

        // Mutators
    void   stand();
    void   moveOrSwat(int dir);
    void   setDead();

  private:
    Room* m_room;
    int   m_row;
    int   m_col;
    int   m_age;
    bool  m_dead;
};

class Room
{
  public:
        // Constructor/destructor
    Room(int nRows, int nCols);
    ~Room();

        // Accessors
    int     rows() const;
    int     cols() const;
    Player* player() const;
    int     beeCount() const;
    int     numBeesAt(int r, int c) const;
    bool    determineNewPosition(int& r, int& c, int dir) const;
    void    display() const;

        // Mutators
    bool   addBee(int r, int c);
    bool   addPlayer(int r, int c);
    bool   swatBeeAt(int r, int c, int dir);
    bool   moveBees();

  private:
    int     m_rows;
    int     m_cols;
    Player* m_player;
    Bee*    m_bees[MAXBEES];
    int     m_nBees;
};

class Game
{
  public:
        // Constructor/destructor
    Game(int rows, int cols, int nBees);
    ~Game();

        // Mutators
    void play();

  private:
    Room* m_room;
};

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function declarations
///////////////////////////////////////////////////////////////////////////

int decodeDirection(char dir);
int randInt(int min, int max);
void clearScreen();

///////////////////////////////////////////////////////////////////////////
//  Bee implementation
///////////////////////////////////////////////////////////////////////////

Bee::Bee(Room* rp, int r, int c)
{
    // Ensure the bee is created in a valid room
    if (rp == nullptr)
    {
        cout << "***** A bee must be created in some Room!" << endl;
        exit(1);  // Exit the program if the room is null
    }

    // Ensure the bee's coordinates are within valid bounds in the room
    if (r < 1  ||  r > rp->rows()  ||  c < 1  ||  c > rp->cols())
    {
        cout << "***** Bee created with invalid coordinates (" << r << ","
             << c << ")!" << endl;
        exit(1);  // Exit the program if the coordinates are out of bounds
    }

    // Initialize bee's state
    m_room = rp;  // Assign the room where the bee will exist
    m_row = r;     // Set the bee's row
    m_col = c;     // Set the bee's column
    m_health = INITIAL_BEE_HEALTH;  // Set initial health
}

int Bee::row() const { return m_row; }  // Return bee's current row
int Bee::col() const { return m_col; }  // Return bee's current column

void Bee::move()
{
    // Randomly determine direction for the bee to move
    int dir = randInt(0, NUMDIRS - 1);  // Random direction
    int newR = m_row, newC = m_col;     // Store bee's current position

    // Try to move the bee to a new position
    if (m_room->determineNewPosition(newR, newC, dir))
    {
        m_row = newR;  // Update bee's row
        m_col = newC;  // Update bee's column
    }
}

bool Bee::getSwatted(int dir)
{
    // Decrease health by 1 and check if the bee dies after a second swat
    if (--m_health == 0)
        return true;  // Bee dies after second swat
    
    int newR = m_row, newC = m_col;
    
    // Check if the bee can move after being swatted
    if (!m_room->determineNewPosition(newR, newC, dir))
        return true;  // Bee dies if it can't move

    // Update bee's position if it can move
    m_row = newR;
    m_col = newC;
    return false;  // Bee survives and moves
}

///////////////////////////////////////////////////////////////////////////
//  Player implementations
///////////////////////////////////////////////////////////////////////////

Player::Player(Room* rp, int r, int c) : m_room(rp), m_row(r), m_col(c), m_age(0), m_dead(false) {}

int Player::row() const { return m_row; }  // Return player's current row
int Player::col() const { return m_col; }  // Return player's current column
int Player::age() const { return m_age; }  // Return player's age (number of steps)
bool Player::isDead() const { return m_dead; }  // Check if the player is dead

void Player::stand() { m_age++; }  // Increment age (number of steps taken)

void Player::moveOrSwat(int dir)
{
    m_age++;  // Increment age on every action (move or swat)
    int newR = m_row, newC = m_col;

    // Determine if the player can move in the given direction
    if (m_room->determineNewPosition(newR, newC, dir))
    {
        // If there are bees at the new position, swat the bee
        if (m_room->numBeesAt(newR, newC) > 0)
            m_room->swatBeeAt(newR, newC, dir);
        else
        {
            // Otherwise, just move the player to the new position
            m_row = newR;
            m_col = newC;
        }
    }
}

void Player::setDead() { m_dead = true; }  // Mark the player as dead

///////////////////////////////////////////////////////////////////////////
//  Room implementations
///////////////////////////////////////////////////////////////////////////

Room::Room(int nRows, int nCols) : m_rows(nRows), m_cols(nCols), m_player(nullptr), m_nBees(0) {}

Room::~Room()
{
    delete m_player;  // Clean up player memory
    // Clean up memory for all bees in the room
    for (int i = 0; i < m_nBees; i++)
        delete m_bees[i];
}

int Room::rows() const { return m_rows; }  // Return the number of rows in the room
int Room::cols() const { return m_cols; }  // Return the number of columns in the room
Player* Room::player() const { return m_player; }  // Return the player in the room
int Room::beeCount() const { return m_nBees; }  // Return the number of bees in the room

int Room::numBeesAt(int r, int c) const
{
    int count = 0;
    for (int i = 0; i < m_nBees; i++)
        if (m_bees[i]->row() == r && m_bees[i]->col() == c)
            count++;  // Count the number of bees at position (r, c)
    return count;
}

bool Room::determineNewPosition(int& r, int& c, int dir) const
{
    // Check if the new position is within bounds for the specified direction
    switch (dir)
    {
        case UP:    if (r > 1) r--; else return false; break;
        case DOWN:  if (r < m_rows) r++; else return false; break;
        case LEFT:  if (c > 1) c--; else return false; break;
        case RIGHT: if (c < m_cols) c++; else return false; break;
        default: return false;  // Invalid direction
    }
    return true;  // Position is valid
}

void Room::display() const
{
    // Create a grid for the room and initialize all cells to '.'
    char grid[MAXROWS][MAXCOLS];
    for (int r = 0; r < rows(); r++)
        for (int c = 0; c < cols(); c++)
            grid[r][c] = '.';

    // Place bees on the grid, marking each with 'B'
    for (int i = 0; i < m_nBees; i++)
    {
        int r = m_bees[i]->row() - 1, c = m_bees[i]->col() - 1;
        if (grid[r][c] == '.')
            grid[r][c] = 'B';  // Place a single bee
        else if (isdigit(grid[r][c]) && grid[r][c] < '9')
            grid[r][c]++;  // Increment bee count if more than one bee at the same spot
        else
            grid[r][c] = '9';  // Cap bee count at '9' (max possible)
    }

    // Place the player on the grid, marking with '@' or '*' based on their state
    if (m_player != nullptr)
    {
        char& cell = grid[m_player->row() - 1][m_player->col() - 1];
        cell = (cell == '.') ? '@' : '*';  // '@' for player, '*' for dead player
    }

    // Display the grid
    clearScreen();
    for (int r = 0; r < rows(); r++)
    {
        for (int c = 0; c < cols(); c++)
            cout << grid[r][c];
        cout << endl;
    }

    // Show information about the room state
    cout << "There are " << beeCount() << " bees remaining." << endl;
    if (m_player == nullptr)
        cout << "There is no player." << endl;
    else if (m_player->isDead())
        cout << "The player is dead." << endl;
    else
        cout << "The player has lasted " << m_player->age() << " steps." << endl;
}

bool Room::addBee(int r, int c)
{
    // Add a Bee to the room if there is space
    if (m_nBees >= MAXBEES)
        return false;  // Return false if no more bees can be added
    m_bees[m_nBees++] = new Bee(this, r, c);  // Add the Bee to the array
    return true;  // Return true if the bee was successfully added
}

bool Room::addPlayer(int r, int c)
{
    if (m_player != nullptr)
        return false; // Only one player can exist in the room

    m_player = new Player(this, r, c);
    return true; // Player successfully added
}

bool Room::swatBeeAt(int r, int c, int dir)
{
    for (int i = 0; i < m_nBees; i++)
    {
        if (m_bees[i]->row() == r && m_bees[i]->col() == c)
        {
            if (m_bees[i]->getSwatted(dir))
            {
                delete m_bees[i];
                for (int j = i; j < m_nBees - 1; j++)
                    m_bees[j] = m_bees[j + 1];
                m_nBees--;
                return true; // Bee was swatted and removed
            }
            return false; // Bee was swatted but not killed
        }
    }
    return false; // No bee at the specified location
}

bool Room::moveBees()
{
    for (int k = 0; k < m_nBees; k++)
    {
        // Have the k-th bee in the room make one move
        m_bees[k]->move();  // No argument is passed

        // Check if the bee's new position matches the player's position
        if (m_bees[k]->row() == m_player->row() && m_bees[k]->col() == m_player->col())
        {
            // If a bee is in the same position as the player, the player dies
            m_player->setDead();
            return false; // Player is dead, return false
        }
    }

    // Return true if the player is still alive
    return !m_player->isDead();
}


///////////////////////////////////////////////////////////////////////////
//  Game implementations
///////////////////////////////////////////////////////////////////////////

Game::Game(int rows, int cols, int nBees)
{
    if (nBees < 0)
    {
        cout << "***** Cannot create Game with negative number of bees!" << endl;
        exit(1);
    }
    if (nBees > MAXBEES)
    {
        cout << "***** Trying to create Game with " << nBees
             << " bees; only " << MAXBEES << " are allowed!" << endl;
        exit(1);
    }
    if (rows == 1  &&  cols == 1  &&  nBees > 0)
    {
        cout << "***** Cannot create Game with nowhere to place the bees!" << endl;
        exit(1);
    }

        // Create room
    m_room = new Room(rows, cols);

        // Add player
    int rPlayer = randInt(1, rows);
    int cPlayer = randInt(1, cols);
    m_room->addPlayer(rPlayer, cPlayer);

      // Populate with bees
    while (nBees > 0)
    {
        int r = randInt(1, rows);
        int c = randInt(1, cols);
          // Don't put a bee where the player is
        if (r == rPlayer  &&  c == cPlayer)
            continue;
        m_room->addBee(r, c);
        nBees--;
    }
}

Game::~Game()
{
    delete m_room;
}

void Game::play()
{
    m_room->display();
    Player* player = m_room->player();
    if (player == nullptr)
        return;
    while ( ! player->isDead()  &&  m_room->beeCount() > 0)
    {
        cout << endl;
        cout << "Move (u/d/l/r/q or nothing): ";
        string action;
        getline(cin,action);
        if (action.size() == 0)  // player stands
            player->stand();
        else
        {
            switch (action[0])
            {
              default:   // if bad move, nobody moves
                cout << '\a' << endl;  // beep
                continue;
              case 'q':
                return;
              case 'u':
              case 'd':
              case 'l':
              case 'r':
                player->moveOrSwat(decodeDirection(action[0]));
                break;
            }
        }
        m_room->moveBees();
        m_room->display();
    }
}

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function implementations
///////////////////////////////////////////////////////////////////////////

int decodeDirection(char dir)
{
    switch (dir)
    {
      case 'u':  return UP;
      case 'd':  return DOWN;
      case 'l':  return LEFT;
      case 'r':  return RIGHT;
    }
    return -1;  // bad argument passed in!
}

  // Return a uniformly distributed random int from min to max, inclusive
int randInt(int min, int max)
{
    if (max < min)
        swap(max, min);
    static random_device rnd;
    static default_random_engine generator(rnd());
    uniform_int_distribution<> distro(min, max);
    return distro(generator);
}

///////////////////////////////////////////////////////////////////////////
//  main()
///////////////////////////////////////////////////////////////////////////

// Tests
#include <type_traits>
#include <cassert>

#define CHECKTYPE(c, f, r, a)  \
    static_assert(std::is_same<decltype(&c::f), r (c::*)a>::value, \
       "FAILED: You changed the type of " #c "::" #f);  \
    { [[gnu::unused]] auto p = static_cast<r(c::*)a>(&c::f); }

void thisFunctionWillNeverBeCalled()
{
      // If the student deleted or changed the interfaces to the public
      // functions, this won't compile.  (This uses magic beyond the scope
      // of CS 31.)

    Bee(static_cast<Room*>(0), 1, 1);
    CHECKTYPE(Bee, row, int, () const);
    CHECKTYPE(Bee, col, int, () const);
    CHECKTYPE(Bee, move, void, ());
    CHECKTYPE(Bee, getSwatted, bool, (int));

    Player(static_cast<Room*>(0), 1, 1);
    CHECKTYPE(Player, row, int, () const);
    CHECKTYPE(Player, col, int, () const);
    CHECKTYPE(Player, age, int, () const);
    CHECKTYPE(Player, isDead, bool, () const);
    CHECKTYPE(Player, stand, void, ());
    CHECKTYPE(Player, moveOrSwat, void, (int));
    CHECKTYPE(Player, setDead, void, ());

    Room(1, 1);
    CHECKTYPE(Room, rows, int, () const);
    CHECKTYPE(Room, cols, int, () const);
    CHECKTYPE(Room, player, Player*, () const);
    CHECKTYPE(Room, beeCount, int, () const);
    CHECKTYPE(Room, numBeesAt, int, (int, int) const);
    CHECKTYPE(Room, determineNewPosition, bool, (int&, int&, int) const);
    CHECKTYPE(Room, display, void, () const);
    CHECKTYPE(Room, addBee, bool, (int, int));
    CHECKTYPE(Room, addPlayer, bool, (int, int));
    CHECKTYPE(Room, swatBeeAt, bool, (int, int, int));
    CHECKTYPE(Room, moveBees, bool, ());

    Game(1, 1, 1);
    CHECKTYPE(Game, play, void, ());
}

void doBasicTests()
{
    {
        Room mate(10, 20);
        assert(mate.addPlayer(2, 6));
        Player* pp = mate.player();
        assert(pp->row() == 2  &&  pp->col() == 6  && ! pp->isDead());
        pp->moveOrSwat(UP);
        assert(pp->row() == 1  &&  pp->col() == 6  && ! pp->isDead());
        pp->moveOrSwat(UP);
        assert(pp->row() == 1  &&  pp->col() == 6  && ! pp->isDead());
        pp->setDead();
        assert(pp->row() == 1  &&  pp->col() == 6  && pp->isDead());
    }
    {
        Room service(2, 2);
        assert(service.addPlayer(1, 1));
        assert(service.addBee(2, 2));
        Player* pp = service.player();
        assert(service.moveBees());
        assert( ! pp->isDead());
        for (int k = 0; k < 1000  &&  ! pp->isDead()  &&  service.moveBees(); k++)
            assert(service.numBeesAt(1, 1) == 0);
        assert(pp->isDead()  &&  service.numBeesAt(1, 1) == 1);
    }
    {
        Room andBoard(2, 6);
        assert(andBoard.addPlayer(2, 1));
        assert(andBoard.addBee(2, 3));
        assert(andBoard.addBee(1, 1));
        assert(andBoard.addBee(1, 6));
        Player* pp = andBoard.player();
        pp->moveOrSwat(RIGHT);
        assert(andBoard.beeCount() == 3  &&  andBoard.numBeesAt(2, 3) == 1);
        pp->moveOrSwat(RIGHT);
        assert(andBoard.beeCount() == 3  &&  andBoard.numBeesAt(2, 4) == 1);
        pp->moveOrSwat(RIGHT);
        assert(andBoard.beeCount() == 3  &&  andBoard.numBeesAt(2, 4) == 1);
        pp->moveOrSwat(RIGHT);
        assert(andBoard.beeCount() == 2  &&  andBoard.numBeesAt(2, 4) == 0   &&  andBoard.numBeesAt(2, 5) == 0);
        andBoard.addBee(1, 3);
        assert(andBoard.beeCount() == 3  &&  andBoard.numBeesAt(1, 3) == 1);
        pp->moveOrSwat(UP);
        assert(andBoard.beeCount() == 2  &&  andBoard.numBeesAt(1, 3) == 0);
          // If the program crashes after leaving this compound statement, you
          // are probably messing something up when you delete a dead Bee
          // (or you have mis-coded the destructor).
          //
          // Draw a picture of your m_bees array before attcking
          // and also note the values of m_nBees or any other variables you
          // might have that are involved with the number of Bees.  Trace
          // through your code step by step as the Bees die and are
          // removed, updating the picture according to what the code says, not
          // what you want it to do.  If you don't see a problem then, try
          // tracing through the destruction of the room.
          //
          // If you execute the code, use the debugger to check on the values
          // of key variables at various points.  If you didn't try to learn
          // to use the debugger, insert statements that write the values of
          // key variables to cerr so you can trace the execution of your code
          // and see the first place where something has gone amiss.  (Comment
          // out the call to clearScreen in Room::display so that your output
          // doesn't disappear.)
    }
    cout << "Passed all basic tests" << endl;
}

int main()
{
    // doBasicTests(); // Remove this line after completing test.
        // return 0;       // Remove this line after completing test.
    // Create a game
      // Use this instead to create a mini-game:   Game g(3, 4, 2);
    Game g(7, 8, 25);

      // Play the game
    g.play();
}

///////////////////////////////////////////////////////////////////////////
//  clearScreen implementation
///////////////////////////////////////////////////////////////////////////

// DO NOT MODIFY OR REMOVE ANYTHING BETWEEN HERE AND THE END OF THE FILE!!!
// THE CODE IS SUITABLE FOR VISUAL C++, XCODE, AND g++/g31 UNDER LINUX.

// Note to Xcode users:  clearScreen() will just write a newline instead
// of clearing the window if you launch your program from within Xcode.
// That's acceptable.  (The Xcode output window doesn't have the capability
// of being cleared.)

#ifdef _WIN32

#pragma warning(disable : 4005)
#include <windows.h>

void clearScreen()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    COORD upperLeft = { 0, 0 };
    DWORD dwCharsWritten;
    FillConsoleOutputCharacter(hConsole, TCHAR(' '), dwConSize, upperLeft,
                                                        &dwCharsWritten);
    SetConsoleCursorPosition(hConsole, upperLeft);
}

#else  // not _WIN32

#include <iostream>
#include <cstring>
#include <cstdlib>

void clearScreen()  // will just write a newline in an Xcode output window
{
    static const char* term = getenv("TERM");
    if (term == nullptr  ||  strcmp(term, "dumb") == 0)
        cout << endl;
    else
    {
        static const char* ESC_SEQ = "\x1B[";  // ANSI Terminal esc seq:  ESC [
        cout << ESC_SEQ << "2J" << ESC_SEQ << "H" << flush;
    }
}

#endif
