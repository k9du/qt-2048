#include "mainwindow.hh"
#include "ui_mainwindow.h"
#include "gameboard.hh"
#include "numbertile.hh"
#include <cmath>
#include <string>
#include <QKeyEvent>
#include <QPalette>
#include <QPixmap>
#include <QFile>
#include <QTextStream>

using uint = unsigned int;
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Read photos
    readPhotosIntoMap();

    // Initialize gameboard
    gameBoard = new GameBoard;

    // Create a pointer for the graphicsscene
    scene = new QGraphicsScene(this);

    // Setup margins for the scene
    int leftMargin = 14;
    int topMargin = 20;

    // Set geometry and add the scene
    ui->gameGraphicsView->setGeometry(leftMargin, topMargin,
                                      BOX_SIZE+3, BOX_SIZE+3);
    ui->gameGraphicsView->setScene(scene);

    // Disable moving capabilities
    disableBoard(true);

    // Set maximum of seed and target
    ui->seedSpinBox->setMinimum(0);
    ui->seedSpinBox->setMaximum(100000);

    ui->targetSpinBox->setMinimum(2);
    ui->targetSpinBox->setValue(11);
    ui->targetSpinBox->setMaximum(SIZE*SIZE);

    // Set scene size
    scene->setSceneRect(0,0,BOX_SIZE,BOX_SIZE);

    // Disable since the game isn't on
    resetButtonChange(true);

    // Create a pointer for the timer
    timer = new QTimer(this);
    time = 0;

    // Timer customization
    ui->minLcdNumber->display(0);
    ui->secLcdNumber->display(0);
    ui->minLcdNumber->setStyleSheet("background-color: darkorange");
    ui->secLcdNumber->setStyleSheet("background-color: orange");
    connect(timer, &QTimer::timeout, this, &MainWindow::clock);

    // Create board
    createGameBoard();
}

MainWindow::~MainWindow()
{
    delete gameBoard;
    delete ui;
}

void MainWindow::initializeGameBoard()
{
    // Clear the base gameboard
    gameBoard->clear_game();
}

void MainWindow::createGameBoard() const
{
    // Go through every coordinate and add a defined slotSize sized
    // rectanlge to represent the empty squares on the board
    for ( uint y = 0; y < SIZE; ++y ) {
        for ( uint x = 0; x < SIZE; ++x ) {
            scene->addRect(x*slotSize, y*slotSize, slotSize, slotSize);
        }
    }
}

void MainWindow::emptyGameBoard()
{
    // Go through the clearing vector and delete and null all
    // labels, at the end clear the vector
    for ( uint i = 0; i < tempLabels.size(); ++i ) {
        delete tempLabels.at(i);
        tempLabels.at(i) = nullptr;
    }
    tempLabels.clear();
}

void MainWindow::on_playPushButton_clicked()
{
    playButtonPressed++;

    // Let the user move and enable buttons
    disableBoard(false);
    resetButtonChange(false);

    gameIsGoingOn = true;

    // Start the timer
    pauseTimer(false);

    // Set the scores
    ui->currentScoreTextBrowser->setText(QString::number(gameScore));
    ui->highscoreTextBrowser->setText(QString::number(gameHighscore));

    // Get the seed and target values
    targetValue = ui->targetSpinBox->value();

    // Correct the value
    targetValueCorrected = pow(2,targetValue);
    ui->targetValueTextBrowser->setText(QString::number(targetValueCorrected));

    // Only the first time pressed we need the 'init_empty()' method
    if ( playButtonPressed == 1 ) {
        gameBoard->init_empty();
    }

    // Get the seed and fill the board
    seedValue = ui->seedSpinBox->value();
    gameBoard->fill(seedValue);

    for ( uint y = 0; y < SIZE; ++y ) {
        for ( uint x = 0; x < SIZE; ++x ) {

            // Check if we found coordinates holding a value
            if ( !gameBoard->get_item(make_pair(y,x))->is_empty() ) {

                // Create a new label
                QLabel* new_label = new QLabel();

                // Push back into the clearing vector
                tempLabels.push_back(new_label);

                // Add the label to the scene
                scene->addWidget(new_label);
                new_label->move(x*slotSize, y*slotSize);

                // Center the label
                new_label->move(x*slotSize, y*slotSize);

                // Get the value of the tile, and then the corresponding photo
                // and add the photo to table, on the right spot.
                int intValue = gameBoard->get_item(make_pair(y,x))
                                                   ->get_value();

                QPixmap pix(photoIconsByValue.at(intValue));
                new_label->setPixmap(pix.scaled(slotSize, slotSize,
                                                Qt::KeepAspectRatio));
            }
        }
    }
}


void MainWindow::on_resetPushButton_clicked()
{
    resetGame();
}

void MainWindow::resetGame()
{
    gameIsGoingOn = false;

    // Empty the scene, and the actual gameboard
    emptyGameBoard();
    initializeGameBoard();

    // Disable moving and basicly return the same view as in the beginning
    disableBoard(true);
    resetButtonChange(true);
    resetAllNumbers();
}

void MainWindow::updateGameBoard()
{
    // Empty the board first
    emptyGameBoard();

    // Find out whats new and update accordingly
    for ( uint y = 0; y < SIZE; ++y ) {
        for ( uint x = 0; x < SIZE; ++x ) {

            // Check if certain spot on the board holds a value
            if ( not gameBoard->get_item(make_pair(y,x))->is_empty() ) {

                // Create and add a new label to the scene
                QLabel* new_label = new QLabel();
                scene->addWidget(new_label);

                // Place the label
                new_label->move(x*slotSize, y*slotSize);

                // Find the right picture and add it to the label
                int intValue = gameBoard->get_item(make_pair(y,x))
                                                   ->get_value();

                QPixmap pix(photoIconsByValue.at(intValue));
                new_label->setPixmap(pix.scaled(slotSize, slotSize,
                                                Qt::KeepAspectRatio));

                // Push the label in to the clearing vector
                tempLabels.push_back(new_label);
            }
        }
    }
}

void MainWindow::moveBoard(const pair<int, int> direction)
{
    // Win check
    if ( gameBoard->move(direction, targetValueCorrected) ) {
        updateGameBoard();
        pauseTimer(true);
        winningMessageBox();
        return;
    }

    // Loss check
    if ( !gameBoard->is_full() ) {
        pointsUpdater();
        gameBoard->new_value();
        updateGameBoard();
    } else {
        pauseTimer(true);
        lossMessageBox();
        return;
    }
}

void MainWindow::pauseTimer(bool toBePaused)
{
    if ( toBePaused ) {
        // We are trying to pause
        timer->stop();
    } else {
        // Else start the timer again
        timer->start(1000);
    }
}

void MainWindow::clock()
{
    time++;
    ui->minLcdNumber->display(time/60);
    ui->secLcdNumber->display(time%60);
}

int MainWindow::maxValueOfBoard()
{
    // Go through board and find the largest value
    // by comparing every value on the board
    for ( uint y = 0; y < SIZE; ++y ) {
        for ( uint x = 0; x < SIZE; ++x ) {
            if ( gameBoard->get_item(make_pair(y,x))->get_value() > largestTile ) {
                largestTile = gameBoard->get_item(make_pair(y,x))->get_value();
            }
        }
    }
    return largestTile;
}

void MainWindow::pointsUpdater()
{
    // Get max value
    int scoreToAdd = maxValueOfBoard();

    // Get current score and add to it
    QString textScore = ui->currentScoreTextBrowser->toPlainText();
    gameScore = stoi(textScore.toStdString());
    gameScore += scoreToAdd;

    // Update the score
    ui->currentScoreTextBrowser->setText(QString::number(gameScore));

    // Check if we are going to update highscore also
    QString textHighscore = ui->highscoreTextBrowser->toPlainText();
    gameHighscore = stoi(textHighscore.toStdString());

    if ( gameScore > gameHighscore ) {
        // Means we found a new highscore so update it
        gameHighscore = gameScore;
        ui->highscoreTextBrowser->setText(QString::number(gameHighscore));
    }
}

void MainWindow::disableBoard(bool disable)
{
    if ( disable ) {
        isPaused = true;
        disableArrowButtons(true);
    } else {
        isPaused = false;
        disableArrowButtons(false);
    }
}

void MainWindow::resetButtonChange(bool reset)
{
    // The buttons and spinboxes work as followed, either the pause and reset
    // button are enabled and the rest disabled, and vice versa.
    ui->seedSpinBox->setEnabled(reset);
    ui->targetSpinBox->setEnabled(reset);
    ui->playPushButton->setEnabled(reset);
    ui->resetPushButton->setEnabled(!reset);
    ui->pausePushButton->setEnabled(!reset);
}

void MainWindow::pauseButtonChange(bool pause)
{
    // If false, means we are trying to continue
    // in the game, so start the timer and else stop it
    if ( !pause ) {
        timer->start(1000);
    } else {
        timer->stop();
    }

    // Update the pause state according to the parameter
    isPaused = pause;
    disableBoard(pause);
    ui->resetPushButton->setDisabled(!pause);
}

void MainWindow::disableArrowButtons(bool disable)
{
    ui->arrowDownPushButton->setDisabled(disable);
    ui->arrowUpPushButton->setDisabled(disable);
    ui->arrowRightPushButton->setDisabled(disable);
    ui->arrowLeftPushButton->setDisabled(disable);
}

void MainWindow::resetAllNumbers()
{
    // Reset largest tile
    largestTile = 0;

    // Default seed and target values
    ui->targetSpinBox->setValue(11);
    ui->targetValueTextBrowser->setText("");
    ui->seedSpinBox->setValue(0);

    // Set timer to default values
    time = 0;
    timer->stop();
    ui->minLcdNumber->display(0);
    ui->secLcdNumber->display(0);

    // Set starting score
    gameScore = 0;
    ui->currentScoreTextBrowser->setText(QString::number(gameScore));
}

void MainWindow::pauseGameBoard()
{
    bool pause;

    // Pause only works if there is a game going on
    if ( gameIsGoingOn ) {

        // If we are trying to continue
        if ( isPaused ) {

            // Change button text back to pause / keskeytä
            if ( !isFinnish ) {
                ui->pausePushButton->setText("Pause");
                ui->actionPause->setText("Pause");
            } else {
                ui->pausePushButton->setText("Keskeytä");
                ui->actionPause->setText("Keskeytä");
            }

            // Change the state of the pause
            pause = false;
            pauseButtonChange(pause);
        } else {

            // Update pause button text back to unpause / jatka
            if ( !isFinnish ) {
                ui->pausePushButton->setText("Unpause");
                ui->actionPause->setText("Unpause");
            } else {
                ui->pausePushButton->setText("Jatka");
                ui->actionPause->setText("Jatka");
            }

            // Change the state of the pause
            pause = true;
            pauseButtonChange(pause);
        }
    }
}

void MainWindow::winningMessageBox()
{
    QMessageBox *win = new QMessageBox(this);

    // Get the time values and the points of the round
    QString mins = QString::number(ui->minLcdNumber->value());
    QString secs = QString::number(ui->secLcdNumber->value());
    QString points = QString::number(gameScore);
    QString winningMessage;

    // Create the winning messages according to language
    if ( !isFinnish ) {
        winningMessage = "You won with a time of " + mins + " minute(s)\nand "
                         + secs + " second(s)!\n" +
                         "Your total points this game was "+ points +
                         "!\nDo you wish to continue?";
    } else {
        winningMessage = "Voitit ajalla " + mins + " minuutti(a)\nja " +
                         secs + " sekuntia!\n" + "Pisteet tällä kertaa " +
                         points + "!\nHaluatko jatkaa?";
    }

    // Creating the messagebox info
    reply = win->question(0, "2048", winningMessage,
                          QMessageBox::Yes|QMessageBox::No);

    // If the user presses yes, start a new game
    // and if no, close the window.
    // In any case delete the memory taken by the messagebox.
    if ( reply == QMessageBox::Yes ) {
        delete win;
        win = nullptr;
        resetGame();
    } else if ( reply == QMessageBox::No ) {
        delete win;
        win = nullptr;
        QMainWindow::close();
    }
}

void MainWindow::instructionsMessageBox()
{
    pauseTimer(true);
    QMessageBox *instructions = new QMessageBox(this);

    // We have different instructions according to language,
    // so choose the file name we try to open according to that
    QString filename;
    if ( !isFinnish ) {
        filename = "instructions.txt";
    } else {
        filename = "instructionsFinnish.txt";
    }

    // Open the instructions file
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);

    // String variable to hold all the text of the file
    QString instructionsText;
    instructionsText = in.readAll();

    // Messagebox info
    reply = instructions->information(0, "2048", instructionsText,
                                      QMessageBox::Ok);

    // When the 'Ok' button is pressed
    if ( reply == QMessageBox::Ok ) {
        if ( gameIsGoingOn ) {
            pauseTimer(false);
        }
        delete instructions;
        instructions = nullptr;
    }
    file.close();
}

void MainWindow::lossMessageBox()
{
    // New messagebox
    QMessageBox *loss = new QMessageBox(this);

    // The message depends on the language so check
    // and create the message accordingly
    QString losingMessage;
    if ( !isFinnish ) {
        losingMessage = "You lost.\nWould you like to try again?";
    } else {
        losingMessage = "Hävisit.\nHaluatko yrittää uudestaan?";
    }

    // Create messagebox info
    reply = loss->question(0, "2048", losingMessage,
                  QMessageBox::Yes|QMessageBox::No);

    // If user pressed yes, start a new game
    if ( reply == QMessageBox::Yes ) {
        delete loss;
        loss = nullptr;
        resetGame();

    // If user pressed no, exit
    } else if ( reply == QMessageBox::No ) {
        delete loss;
        loss = nullptr;
        QMainWindow::close();
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    // Only when the game is going on we want to
    // be able to move
    if ( !isPaused ) {
        if (event->key() == Qt::Key_Down) {
            moveBoard(DOWN_DIRECTION);
        } else if ( event->key() == Qt::Key_Left) {
            moveBoard(LEFT_DIRECTION);
        } else if ( event->key() == Qt::Key_Right) {
            moveBoard(RIGHT_DIRECTION);
        } else if ( event->key() == Qt::Key_Up) {
            moveBoard(UP_DIRECTION);
        }
    }
}

void MainWindow::on_pausePushButton_clicked()
{
    pauseGameBoard();
}

void MainWindow::on_arrowUpPushButton_clicked()
{
    moveBoard(UP_DIRECTION);
}


void MainWindow::on_arrowRightPushButton_clicked()
{
    moveBoard(RIGHT_DIRECTION);
}


void MainWindow::on_arrowDownPushButton_clicked()
{
    moveBoard(DOWN_DIRECTION);
}


void MainWindow::on_arrowLeftPushButton_clicked()
{
    moveBoard(LEFT_DIRECTION);
}

void MainWindow::on_actionQuit_triggered()
{
    QMainWindow::close();
}

void MainWindow::on_actionReset_triggered()
{
    // Reset only works when not paused
    if ( !isPaused ) {
        resetGame();
    }
}

void MainWindow::on_actionPause_triggered()
{
    pauseGameBoard();
}


void MainWindow::on_actionInstructions_triggered()
{
    instructionsMessageBox();
}

void MainWindow::on_actionEnglish_triggered()
{
    // Change all buttons and labels back to english.
    // Only do it if isFinnish is true, so that we don't
    // do useless work
    if ( isFinnish ) {
        isFinnish = false;
        ui->playPushButton->setText("Play");
        ui->resetPushButton->setText("Reset");
        ui->closePushButton->setText("Close");
        ui->pausePushButton->setText("Pause");
        ui->secLabel->setText("sec");
        ui->pointsLabel->setText("Your points:");
        ui->highScoreLabel1->setText("Your highscore:");
        ui->highScoreLabel2->setText("(this session)");
        ui->seedValueLabel->setText("Seed value:");
        ui->targetValueLabel1->setText("Target value:");
        ui->targetValueLabel2->setText("(as a power of 2)");
        ui->actionInstructions->setText("Instructions");
        ui->actionPause->setText("Pause");
        ui->actionQuit->setText("Quit");
        ui->actionReset->setText("Reset");
        ui->menuLanguage->setTitle("Language");
        ui->menuHelp->setTitle("Help");
        ui->menuSettings->setTitle("Settings");
    }
}


void MainWindow::on_actionSuomi_triggered()
{
    // Change all to Finnish unless we are already using
    // the Finnish language
    if ( !isFinnish  ) {
        isFinnish = true;
        ui->playPushButton->setText("Pelaa	");
        ui->resetPushButton->setText("Uusi peli");
        ui->closePushButton->setText("Sulje");
        ui->pausePushButton->setText("Keskeytä");
        ui->secLabel->setText("sek");
        ui->pointsLabel->setText("Pisteesi:");
        ui->highScoreLabel1->setText("Ennätyksesi:");
        ui->highScoreLabel2->setText("(tällä kertaa)");
        ui->seedValueLabel->setText("Siemenluku:");
        ui->targetValueLabel1->setText("Tavoite:");
        ui->targetValueLabel2->setText("(2 potenssina)");
        ui->actionInstructions->setText("Ohjeet");
        ui->actionPause->setText("Keskeytä");
        ui->actionQuit->setText("Sulje");
        ui->actionReset->setText("Uusi peli");
        ui->menuLanguage->setTitle("Kieli");
        ui->menuHelp->setTitle("Ohje");
        ui->menuSettings->setTitle("Asetukset");
    }
}

void MainWindow::readPhotosIntoMap()
{
    // Add all photos
    photoIconsByValue = {{2, ":/icons/icons/2.png"},
                         {4, ":/icons/icons/4.png"},
                         {8, ":/icons/icons/8.png"},
                         {16, ":/icons/icons/16.png"},
                         {32, ":/icons/icons/32.png"},
                         {64, ":/icons/icons/64.png"},
                         {128, ":/icons/icons/128.png"},
                         {256, ":/icons/icons/256.png"},
                         {512, ":/icons/icons/512.png"},
                         {1024, ":/icons/icons/1024.png"},
                         {2048, ":/icons/icons/2048.png"},
                         {4096, ":/icons/icons/4096.png"},
                         {8192, ":/icons/icons/8192.png"},
                         {16384, ":/icons/icons/16384.png"},
                         {32768, ":/icons/icons/32768.png"},
                         {65536, ":/icons/icons/65536.png"}};
}

void MainWindow::on_closePushButton_clicked()
{
    this->close();
}
