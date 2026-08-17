#include "gamestate.hpp"

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include "config.hpp"
#include "inputmanager.hpp"
#include "game.hpp"
#include "audio.hpp"
#include "paths.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "utilities.hpp"

/*
 * ====================================
 * Public methods start here
 * ====================================
 */

GameState::GameState (InputManager *manager) : State (manager)
{
    board = nullptr;
    countdown_texture = nullptr;
    gameover_text = nullptr;
    hold_label = nullptr;
    next_label = nullptr;
    tetrominoSprites = nullptr;
    playfieldFrame = nullptr;
}

GameState::~GameState ()
{
    exit();
}

void GameState::initialize ()
{
    currentPhase = GAME_STARTED;
    board = new Board;
    srand(time(0));
    hold_block_first_time = true;
    hold_block_used = false;
    
    // Get random first piece
    nextPiece.piece_type = getRandom(0, 6);
    nextPiece.rotation = 0;                     // Pieces must always start flat according to the offical Tetris guidelines
    createNewPiece(); 
    nextPiece.r = config::next_box_y;
    nextPiece.c = config::next_box_x;

    // Load necessary textures
    countdown_texture = new Texture ();
    gameover_text = new Texture ();
    gameover_text->loadFromText("Game Over!", Game::getInstance()->mRenderer->bigFont, config::default_text_color);
    TTF_Font *label_font = Game::getInstance()->mRenderer->smallFont;
    if (label_font == nullptr)
    {
        label_font = Game::getInstance()->mRenderer->mediumFont;
    }
    hold_label = new Texture ();
    hold_label->loadFromText("HOLD", label_font, config::default_text_color);
    next_label = new Texture ();
    next_label->loadFromText("NEXT", label_font, config::default_text_color);
    tetrominoSprites = new Texture ();
    playfieldFrame = new Texture ();
    tetrominoSprites->loadFromImage(assetPath("tetrominoSprites.png"));
    playfieldFrame->loadFromImage(assetPath("playfieldFrame.png"));

    // Create the right clips sprites
    for (int i = 0; i < 7; i++)
    {
        tetrominoSpriteClips[i].x = config::block_src_size*i;
        tetrominoSpriteClips[i].y = 0;
        tetrominoSpriteClips[i].w = config::block_src_size;
        tetrominoSpriteClips[i].h = config::block_src_size;
    }
    for (int i = 0; i < 4; i++)
    {
        playfieldFrameClips[i].x = config::frame_sprite_size*i;
        playfieldFrameClips[i].y = 0;
        playfieldFrameClips[i].w = config::frame_sprite_size;
        playfieldFrameClips[i].h = config::frame_sprite_size;
    }
    game_just_started = true;
}

void GameState::exit ()
{
    delete board;
    delete countdown_texture;
    delete gameover_text;
    delete hold_label;
    delete next_label;
    delete tetrominoSprites;
    delete playfieldFrame;
    board = nullptr;
    countdown_texture = nullptr;
    gameover_text = nullptr;
    hold_label = nullptr;
    next_label = nullptr;
    tetrominoSprites = nullptr;
    playfieldFrame = nullptr;
}

void GameState::run ()
{
    mInputManager->setRepeatPolicy(RepeatPolicy::gameplay);
    switch (currentPhase)
    {
        case GAME_STARTED:
        {
            if (game_just_started)
            {
                time_snap1 = SDL_GetTicks();
                game_just_started = false;
            }
            unsigned long long ms_passed = SDL_GetTicks() - time_snap1;
            if (ms_passed < 3000)
            {
                while (mInputManager->pollAction())
                {
                    if (mInputManager->isGameExiting())
                    {
                        nextStateID = STATE_EXIT;
                        break;
                    }
                    if (mInputManager->getAction() == Action::back)
                    {
                        Game::getInstance()->popState();
                        break;                                                  // Pop the state only once even if Action::back is pressed twice
                    }
                }
                Game::getInstance()->mRenderer->clearScreen();
                draw();
                int countdown_time = ceil((3000 - ms_passed)/1000.0);           // The time left on the countdown
                if (countdown_time >= 0)
                {
                    countdown_texture->loadFromText(std::to_string(countdown_time), Game::getInstance()->mRenderer->bigFont, config::default_text_color);
                    Game::getInstance()->mRenderer->renderTexture(countdown_texture, config::logical_window_width/2, config::logical_window_height/2);
                }
                Game::getInstance()->mRenderer->updateScreen();
            }
            else
            {
                currentPhase = GAME_PLAYING;
                time_snap1 = SDL_GetTicks();
            }
            break;
        }

        case GAME_PLAYING:
        {
            if (mInputManager->isGameExiting())
            {
                nextStateID = STATE_EXIT;
            }
            else if (!isGameOver())
            {
                while (mInputManager->pollAction())
                {
                    if (mInputManager->getAction() == Action::back)
                    {
                        Game::getInstance()->popState();
                        break;                                                // Pop the state only once even if Action::back is pressed twice
                    }
                    else
                    {
                        handleEvent(mInputManager->getAction());
                    }
                }
                
                time_snap2 = SDL_GetTicks();
                if (time_snap2 - time_snap1 >= config::wait_time)
                {
                    movePieceDown();
                    time_snap1 = SDL_GetTicks();
                }
                Game::getInstance()->mRenderer->clearScreen();
                draw();
                Game::getInstance()->mRenderer->updateScreen();
            }
            else
            {
                // Here the game has finished
                currentPhase = GAME_FINISHED;
            }
            break;
        }

        case GAME_FINISHED:
        {
            if (!mInputManager->isGameExiting())
            {
                while (mInputManager->pollAction() != 0)
                {
                    if (mInputManager->getAction() == Action::back
                        || mInputManager->getAction() == Action::select)
                    {
                        Game::getInstance()->popState();
                        break;
                    }
                }
                Game::getInstance()->mRenderer->clearScreen();
                draw();
                Game::getInstance()->mRenderer->renderTexture(gameover_text, 3+config::logical_window_width/2, config::logical_window_height/2);
                Game::getInstance()->mRenderer->updateScreen();
            }
            else
            {
                // Here the game has exited
                nextStateID = STATE_EXIT;
            }
            break;
        }
    }
}

void GameState::update ()
{
    // We don't use this function in this state, the work is done by handleEvent() 
}

void GameState::draw ()
{
    drawBoard();
    drawCurrentPiece(currentPiece);
    if (!board->isGameOver() && config::ghost_piece_enabled) drawGhostPiece(currentPiece);
    if (!hold_block_first_time) drawHoldPiece(holdPiece);
    drawNextPiece(nextPiece);
    if (hold_label != nullptr)
    {
        hold_label->renderCentered(config::hold_box_x + config::block_size * 2, config::hold_box_y - 24);
    }
    if (next_label != nullptr)
    {
        next_label->renderCentered(config::next_box_x + config::block_size * 2, config::next_box_y - 24);
    }
}

/*
 * ====================================
 * Private methods start here
 * ====================================
 */

bool GameState::isGameOver ()
{
    return board->isGameOver();
}

void GameState::createNewPiece ()
{
    currentPiece.piece_type = nextPiece.piece_type;
    currentPiece.rotation = nextPiece.rotation;
    currentPiece.r = currentPiece.getInitialOffsetR();
    currentPiece.c = config::playfield_width / 2 + currentPiece.getInitialOffsetC();

    for (int i = 0; i < 2; i++)
    { 
        currentPiece.r++;
        if (!board->isPositionLegal(currentPiece))
        {
            currentPiece.r--;
        }
    }
    if (currentPiece.piece_type > 1)
    {
            currentPiece.r++;
            if (!board->isPositionLegal(currentPiece))
            {
                currentPiece.r--;
            }
    }

    nextPiece.piece_type = getRandom(0, 6);
    nextPiece.rotation = 0; // Pieces must always start flat according to the offical Tetris guidelines
}

void GameState::checkState ()
{
    board->storePiece(currentPiece);
    board->clearFullLines();
    if (!board->isGameOver())
    {
        createNewPiece();
    }
    hold_block_used = false;                // We can now use the hold block again
}

void GameState::handleEvent (Action action)
{
    switch(action)
    {
        case Action::move_down:
        {
            currentPiece.r++;
            if (!board->isPositionLegal(currentPiece))
            {
                currentPiece.r--;
                checkState();
            }
            break;
        }

        case Action::move_left:
        {
            currentPiece.c--;
            if (!board->isPositionLegal(currentPiece))
            {
                currentPiece.c++;
            }
            break;
        }

        case Action::move_right:
        {
            currentPiece.c++;
            if (!board->isPositionLegal(currentPiece))
            {
                currentPiece.c--;
            }
            break;
        }

        case Action::drop:
        {
            while (board->isPositionLegal(currentPiece))
            {
                currentPiece.r++;
            }
            currentPiece.r--;
            checkState();
            break;
        }

        case Action::select:
        case Action::move_up:
        case Action::rotate:
        {
            currentPiece.rotation = (currentPiece.rotation + 1) % 4;
            if (!board->isPositionLegal(currentPiece))
            {
                currentPiece.rotation = (currentPiece.rotation + 3) % 4;
            }
            else
            {
                audio::playRotate();
            }
            break;
        }

        case Action::hold:
        {
            if (hold_block_first_time)
            {
                holdPiece = Piece(currentPiece);
                holdPiece.rotation = 0;
                createNewPiece();
                hold_block_first_time = false;
                hold_block_used = true;
            }
            else if (!hold_block_used)
            {
                swap(currentPiece, holdPiece);
                holdPiece.rotation = 0;
                currentPiece.r = currentPiece.getInitialOffsetR();
                currentPiece.c = config::playfield_width / 2 + currentPiece.getInitialOffsetC();

                for (int i = 0; i < 2; i++)
                { 
                    currentPiece.r++;
                    if (!board->isPositionLegal(currentPiece))
                    {
                        currentPiece.r--;
                    }
                }
                if (currentPiece.piece_type > 1)
                {
                        currentPiece.r++;
                        if (!board->isPositionLegal(currentPiece))
                        {
                            currentPiece.r--;
                        }
                }
                hold_block_used = true;
            }
            break;
        }

        case Action::pause:
        {
            currentPhase = GAME_STARTED;
            game_just_started = true;
            Game::getInstance()->pushPaused();
            break;
        }

        default:
            break;
    }
}

void GameState::movePieceDown ()
{
    currentPiece.r++;
    if (!board->isPositionLegal(currentPiece))
    {
        currentPiece.r--;
        checkState();
    }
}

void GameState::drawBoard ()
{
    const int dest = config::block_size;
    const int frame_dest = dest / 2;
    const int overlap = (config::frame_sprite_size - config::frame_width) * dest / config::block_src_size;
    const int n_vert = config::true_playfield_height * dest / frame_dest;
    const int n_horiz = config::playfield_width * dest / frame_dest;
    const int left_x = config::width_to_playfield - frame_dest;
    const int right_x = config::width_to_playfield + dest * config::playfield_width - overlap;
    const int top_y = config::height_to_playfield;
    const int bottom_y = config::height_to_playfield + dest * config::true_playfield_height;

    for (int i = 0; i < n_vert; i++)
    {
        playfieldFrame->render(left_x, top_y + i * frame_dest, &playfieldFrameClips[0], frame_dest, frame_dest);
        playfieldFrame->render(right_x, top_y + i * frame_dest, &playfieldFrameClips[0], frame_dest, frame_dest);
    }
    playfieldFrame->render(left_x, bottom_y - overlap, &playfieldFrameClips[2], frame_dest, frame_dest);
    playfieldFrame->render(config::width_to_playfield + dest * config::playfield_width,
        bottom_y - overlap, &playfieldFrameClips[3], frame_dest, frame_dest);

    for (int i = 0; i < n_horiz; i++)
    {
        playfieldFrame->render(config::width_to_playfield + i * frame_dest, bottom_y,
            &playfieldFrameClips[1], frame_dest, frame_dest);
    }

    for (int row = 0; row < config::playfield_height; row++)
    {
        for (int col = 0; col < config::playfield_width; col++)
        {
            if (!board->isBlockFree(row, col))
            {
                tetrominoSprites->render(config::width_to_playfield + col * dest,
                    config::height_to_playfield + (row-(config::playfield_height-config::true_playfield_height)) * dest,
                    &tetrominoSpriteClips[board->getTetromino(row, col)], dest, dest);
            }
        }
    }
}

void GameState::drawCurrentPiece (Piece p)
{
    const int dest = config::block_size;
    for (int row = 0; row < config::matrix_blocks; row++)
    {
        for (int col = 0; col < config::matrix_blocks; col++)
        {
            if (p.getBlockType(row, col) != 0)
            {
                tetrominoSprites->render(config::width_to_playfield + (col+p.c) * dest,
                    config::height_to_playfield + (row+p.r-(config::playfield_height-config::true_playfield_height)) * dest,
                    &tetrominoSpriteClips[p.piece_type], dest, dest);
            }
        }
    }
}

void GameState::drawNextPiece (Piece p)
{
    const int dest = config::block_size;
    for (int row = 0; row < config::matrix_blocks; row++)
    {
        for (int col = 0; col < config::matrix_blocks; col++)
        {
            if (p.getBlockType(row, col) != 0)
            {
                tetrominoSprites->render(config::next_box_x + col * dest, config::next_box_y + row * dest,
                    &tetrominoSpriteClips[p.piece_type], dest, dest);
            }
        }
    }
}

void GameState::drawHoldPiece (Piece p)
{
    const int dest = config::block_size;
    for (int row = 0; row < config::matrix_blocks; row++)
    {
        for (int col = 0; col < config::matrix_blocks; col++)
        {
            if (p.getBlockType(row, col) != 0)
            {
                tetrominoSprites->render(config::hold_box_x + col * dest, config::hold_box_y + row * dest,
                    &tetrominoSpriteClips[p.piece_type], dest, dest);
            }
        }
    }
}

void GameState::drawGhostPiece (Piece p)
{
    ghostPiece = p;
    while (board->isPositionLegal(ghostPiece))
    {
        ghostPiece.r++;
    }
    ghostPiece.r--;

    tetrominoSprites->setAlphaMode(config::transparency_alpha);
    const int dest = config::block_size;

    for (int row = 0; row < config::matrix_blocks; row++)
    {
        for (int col = 0; col < config::matrix_blocks; col++)
        {
            if (ghostPiece.getBlockType(row, col) != 0)
            {
                tetrominoSprites->render(config::width_to_playfield + (col+ghostPiece.c) * dest,
                    config::height_to_playfield + (row+ghostPiece.r-(config::playfield_height-config::true_playfield_height)) * dest,
                    &tetrominoSpriteClips[ghostPiece.piece_type], dest, dest);
            }
        }
    }

    tetrominoSprites->setAlphaMode(255);
}

int GameState::getRandom (int lower_limit, int upper_limit)
{
    return rand() % (upper_limit - lower_limit + 1) + lower_limit;
}