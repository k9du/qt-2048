/* 2048
 *
 * Description:
 *      This is the GUI-class of the game. It controls the graphic
 * part of the game, by using the given GameBoard and NumberTile
 * classes.
 *      By using slots and methods, the GUI is able to read the
 * GameBoard, and update photos to the board accordingly. The only
 * 'feature' that isn't extra, is the Reset-feature, where we use
 * new methods in the given classes to empty the gameboard.
 *      The actual rules of the game can be found in the instructions.txt
 * file. You may choose the language of the instructions you want to read
 * yourself.
 *
 * Program creator:
 *
 * Name: Kian Moloney
 * E-mail: kian.moloney@tuni.fi
*/

#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include "gameboard.hh"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QLabel>
#include <QString>
#include <QTimer>
#include <QMessageBox>
#include <map>

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    // Manages the start of the game
    void on_playPushButton_clicked();

    // Resets the game
    void on_resetPushButton_clicked();

    // Arrow button action slots for moving
    // in the given direction
    void on_arrowUpPushButton_clicked();
    void on_arrowRightPushButton_clicked();
    void on_arrowDownPushButton_clicked();
    void on_arrowLeftPushButton_clicked();

    // Pauses the game, and the user can't almost anything
    // until unpaused
    void on_pausePushButton_clicked();

    // Simple function to close the game when close button
    // is pressed
    void on_closePushButton_clicked();

    // Quit, reset and pause use the same functions
    // as the buttons
    void on_actionQuit_triggered();
    void on_actionReset_triggered();
    void on_actionPause_triggered();

    // Opens the instructions of the language used
    void on_actionInstructions_triggered();

    // Changes language accordingly
    void on_actionEnglish_triggered();
    void on_actionSuomi_triggered();

private:
    Ui::MainWindow *ui;

    // Scene pointer where we place the game board
    QGraphicsScene* scene;

    // Gameboard object
    GameBoard* gameBoard;

    // Creates the gameboard, as in adds rectangles
    // to the board, where the photos can move
    void createGameBoard() const;

    // Empties the gameboard, by deleting and nulling all pointers
    // to the labels
    void emptyGameBoard();

    // Uses method above to clear the table, and then fetches
    // new values by going through the gameboard, and updates
    // photos according to value
    void updateGameBoard();

    // Moves the board in the given direction
    // takes the direction as an integer pair as a parameter
    void moveBoard(const pair<int,int> direction);

    // Resets the game
    void resetGame();

    // Pauses the gameboard, and stays paused until unpaused again
    void pauseGameBoard();

    // Messagebox method used when the user wins
    void winningMessageBox();

    // Messagebox method used when the instructions action
    // is pressed in the menu
    void instructionsMessageBox();

    // Messagebox method used when the user loses.
    // The user can either quit or try again
    void lossMessageBox();

    // Reads the photos from the resource folder in to a map
    void readPhotosIntoMap();

    // Pauses the timer according to the boolean parameter
    void pauseTimer(bool toBePaused);

    // Simple clock function to update time
    void clock();
    int time = 0;

    // Finds the max value of the board
    int maxValueOfBoard();
    int largestTile = 0;

    // Counts and updates points by using the max value method to find
    // how much to add
    void pointsUpdater();

    // Disables/resumes the moving of the board
    // takes a boolean telling whether to disable or not
    // as parameter
    void disableBoard(bool disable);

    // Changes all the button states according to the state of the game
    // The parameter tells whether to set the buttons to 'reset mode'
    void resetButtonChange(bool reset);

    // Disables/enables the arrow buttons shown on screen
    // takes the boolean as parameter
    void disableArrowButtons(bool disable);

    // Changes all the button states to pause-mode, or resumes
    // according to the parameter
    void pauseButtonChange(bool pause);

    // Changes all the values to default
    void resetAllNumbers();

    // This method calls for the gameboards clear_game() method,
    // when the reset button is pressed
    void initializeGameBoard();

    // These ints are used to store and display the score of the user
    int gameScore = 0;
    int gameHighscore = 0;

    // We only use the 'init_empty()' method of the gameboard the first
    // time around, so this integer is used to keep track of how many
    // times the play button has been pressed so we can use the method
    // when this is equal to 1
    int playButtonPressed = 0;

    // Seed and target values
    int seedValue = 0;
    int targetValue = 0;

    // Create reply button so we can check when
    // the user presses certain button
    QMessageBox::StandardButton reply;

    // Timer
    QTimer* timer;

    // For controlling pause state
    bool isPaused = true;

    // For controlling language
    bool isFinnish = false;

    // For controlling the state of the game
    bool gameIsGoingOn = false;

    // Constant size of the box
    const int BOX_SIZE = 240;

    // The new value 2 as qstring
    QString newValue = QString::number(NEW_VALUE);

    // The size of the side of one box
    int slotSize = BOX_SIZE/SIZE;

    // Vector for emptying the table
    vector<QLabel*> tempLabels;

    // Target value
    int targetValueCorrected = 0;

    // Photo map
    map<int, QString> photoIconsByValue;

    // Different directions for the move method
    const pair<int,int> RIGHT_DIRECTION = make_pair(0,1);
    const pair<int,int> LEFT_DIRECTION = make_pair(0,-1);
    const pair<int,int> UP_DIRECTION = make_pair(-1,0);
    const pair<int,int> DOWN_DIRECTION = make_pair(1,0);

};
#endif // MAINWINDOW_HH
