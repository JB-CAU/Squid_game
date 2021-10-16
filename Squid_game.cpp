#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4819)

#include <bangtal>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

using namespace bangtal;

TimerPtr lobby_timer;
TimerPtr stage1_scene_timer;
TimerPtr stage1_end_timer;
TimerPtr stage1_game_timer;
TimerPtr stage1_trap_timer;
TimerPtr timer;

// start_time은 stage_main(무궁화 꽃이 피었습니다 Sound)가 play되는 순간을 기록하고
// end_time은 사용자로부터 키보드 입력을 받으면 갱신된다.
// 따라서 이 둘은 제한시간 내에 클리어했는지와
// 무궁화 꽃이 피었습니다 멘트 시작한 순간부터 끝나는 순간까지를 기록한다. 5초, 5초마다 2초씩 뒤돌아봄. 7로 나눠서 나머지가 2보다 작으면 탈락.
clock_t start_time;
clock_t end_time;
	
float animationTime = 0.1f; // 장면 전환
float lightCount = 0; // 빛 퍼센트

float game_time_limit = 30;

auto main_scene = Scene::create("메인", "Images/오징어게임.jpg");
auto stage1_gameboard = Scene::create("", "Images/운동장.png");
auto game_over_scene = Scene::create("사망", "Images/게임오버.jpg");
auto stage1_game_over = Scene::create("", "Images/눈/눈 (1).jpg");
auto stage1_main = Sound::create("music/무궁화꽃이피었습니다.mp3");

auto playerX = 610;
auto playerY = 80;

auto trapX = 100;
auto trapY = 200;

auto trap = Object::create("Images/장애물.png", stage1_gameboard, trapX, trapY);

auto player = Object::create("Images/플레이어.png", stage1_gameboard, playerX, playerY);
auto clear_button = Object::create("Images/통과.png", stage1_gameboard, 390, 110, false);

auto gunshot = Sound::create("music/총소리.mp3");

int eye_sceneCount = 1;
int intro_sceneCount = 1;
int stage1_sceneCount = 1;
int trap_Count = 1;

void game_over()
{
	stage1_main->stop();
	stage1_game_timer->stop();
	stage1_trap_timer->stop();
	hideTimer();
	stage1_game_over->enter();
	stage1_end_timer->start();
}

void check_game_over()
{
	if ((end_time - start_time) / CLOCKS_PER_SEC >= game_time_limit) game_over();
}

// 애니메이션 카운트, 플레이어 위치를 디폴트로, 무궁화 멘트 다시 시작
void init_game()
{
	eye_sceneCount = 1;
	intro_sceneCount = 1;
	stage1_sceneCount = 1;
	trap_Count = 1;
	trapX = 100;
	trapY = 200;
	playerX = 610;
	playerY = 80;
	player->locate(stage1_gameboard, playerX, playerY);
	gunshot->stop();
	stage1_trap_timer->start();
	stage1_gameboard->enter();
}

void init_stage1_timer()
{
	start_time = clock();
	showTimer(stage1_game_timer);
	stage1_game_timer->set(30.0f);
	stage1_game_timer->start();
	stage1_main->play();
}

void game_clear()
{
	stage1_main->stop();
	stage1_game_timer->stop();
	hideTimer();
	clear_button->show();
	showMessage("클리어하셨습니다! \n 추후 추가될 게임도 기대해주세요. \n 종료하시려면 클리어버튼을 클릭해주세요.");
	init_game();
}

bool check_touch() // 함정에 닿았는지
{
	if ((playerX <= trapX + 154 && playerX >= trapX - 61) && (playerY <= trapY + 61 && playerY >= trapY - 61 ))
	{
		game_over();
		return true;
	}
	else return false;
}

void check_move()
{
	// clock 함수로 받아온 시간은 밀리초 단위니까 1000으로 나눠준다.
	// math 라이브러리의 fmod를 활용하여 실수 나머지를 구한다. 시간 초 단위는 게임 핵심이라서..
	if (fmod((end_time - start_time) / 1000.0f, 7.0f) <= 2.0f) 
		game_over();
}

bool clear_check() // 결승선의 Y좌표인 570을 플레이어의 Y좌표가 넘어야(완전히 넘어야 함) 통과
{
	if (playerY > 570) return true;
	else return false;
}

int main()
{
	setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);
	setGameOption(GameOption::GAME_OPTION_MESSAGE_BOX_BUTTON, false);
	setGameOption(GameOption::GAME_OPTION_ROOM_TITLE, false);

	clear_button->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)-> bool {
		endGame();
		return 0;
		});

	auto stage1_lobby = Scene::create("무궁화", "Images/무궁화 로비.PNG");

	auto lobby = Scene::create("", "Images/로비.png");
	auto decide_to_join = Scene::create("", "Images/선택.png");

	auto BGM_main = Sound::create("music/Pink Soldiers.mp3");

	auto game_over = Sound::create("music/Lets Go Out Tonight.mp3");
	auto welcome_ment = Sound::create("music/환영멘트.mp3");
	auto stage1_intro = Sound::create("music/무궁화게임_소개.mp3");
	auto select_music = Sound::create("music/투표.mp3");
	auto stage1_rule = Sound::create("music/무궁화 룰 설명.mp3");


	auto game_doll = Object::create("Images/미니게임 인형_뒤.png", stage1_gameboard, 530, 585);
	auto goal_line = Object::create("Images/결승선.png", stage1_gameboard, 0, 570);

	auto game_rule = Object::create("Images/무궁화게임 설명.png", stage1_gameboard, 154, 106, false);
	game_rule->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)-> bool {
		game_rule->hide();
		stage1_rule->stop();
		stage1_trap_timer->start();
		init_stage1_timer();
		return 0;
		});

	auto button_O = Object::create("Images/투명 배경.png", decide_to_join, 350, 270);
	button_O->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)-> bool {
		select_music->stop();
		stage1_lobby->enter();
		return 0;
		});

	auto button_X = Object::create("Images/투명 배경.png", decide_to_join, 800, 270);
	button_X->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)-> bool {
		endGame();
		return 0;
		});

	auto game_start_button = Object::create("Images/시작 버튼.jpg", main_scene, 465, 70);

	auto lobby_next_button = Object::create("Images/다음페이지.png", lobby, 1180, 30);
	lobby_next_button->setScale(0.6f);
	lobby_next_button->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)-> bool {

		lobby_timer->start();
		lobby_next_button->hide();
		return 0;
		});

	auto stage_next_button = Object::create("Images/다음페이지.png", stage1_lobby, 1180, 30);
	stage_next_button->setScale(0.6f);
	stage_next_button->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)-> bool {

		stage_next_button->hide();
		stage1_intro->play();
		stage1_scene_timer->start();
		return 0;
		});

	auto select_next_button = Object::create("Images/다음페이지.png", lobby, 1180, 30, false);
	select_next_button->setScale(0.6f);
	select_next_button->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)-> bool {

		decide_to_join->enter();
		select_music->play();
		return 0;
		});

	auto gameRule_next_button = Object::create("Images/다음페이지.png", stage1_lobby, 1180, 30); // false로
	gameRule_next_button->setScale(0.6f);
	gameRule_next_button->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)-> bool {

		stage1_intro->stop();
		stage1_gameboard->enter();
		game_rule->show();
		stage1_rule->play();
		
		return 0;
		});

	auto game_over_button = Object::create("Images/다음페이지.png", stage1_game_over, 1150, 30, false);
	game_over_button->setScale(0.6f);
	game_over_button->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)-> bool {
		game_over_scene->enter();
		game_over->play();
		showMessage("게임 오버. 다시 해보시겠습니까?");
		return 0;
		});

	auto restart_button = Object::create("Images/다시하기.png", game_over_scene, 1150, 10);
	restart_button->setScale(0.3f);
	restart_button->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)-> bool {
		game_over->stop();
		init_game();
		init_stage1_timer();
		return 0;
		});

	char path[50] = { 0, };


	stage1_end_timer = Timer::create(animationTime);
	stage1_end_timer->setOnTimerCallback([&](auto)->bool
		{
			sprintf(path, "Images/눈/눈 (%d).jpg", eye_sceneCount);
			stage1_game_over->setImage(path);
			eye_sceneCount++;
			if (eye_sceneCount < 27)
			{
				stage1_end_timer->set(animationTime);
				stage1_end_timer->start();
			}
			else
			{
				gunshot->play();
				stage1_game_over->setImage("Images/총2.png");
				game_over_button->show();
			}
			return 0;
		});


	stage1_gameboard->setOnKeyboardCallback([&](ScenePtr stage1_gameboard, KeyCode key, bool pressed)-> bool
		{
			if (key == KeyCode::KEY_RIGHT_ARROW)
			{
				playerX += 3;
				player->locate(stage1_gameboard, playerX, playerY);
				end_time = clock();
				check_game_over();
				check_move();
			
			}
			else if (key == KeyCode::KEY_LEFT_ARROW)
			{
				playerX -= 3;
				player->locate(stage1_gameboard, playerX, playerY);
				end_time = clock();
				if (end_time - start_time >= game_time_limit) check_game_over();
				check_move();
			}
			else if (key == KeyCode::KEY_UP_ARROW)
			{
				playerY += 3;
				player->locate(stage1_gameboard, playerX, playerY);
				end_time = clock();
				if (end_time - start_time >= game_time_limit) check_game_over();
				if (clear_check()) game_clear();
				check_move();
			}
			else if (key == KeyCode::KEY_DOWN_ARROW)
			{
				playerY -= 3;
				player->locate(stage1_gameboard, playerX, playerY);
				end_time = clock();
				if (end_time - start_time >= game_time_limit) check_game_over();
				check_move();
			}
			return 0;
		});


	lobby_timer = Timer::create(animationTime);
	lobby_timer->setOnTimerCallback([&](auto)->bool
		{
			sprintf(path, "Images/인트로/인트로 (%d).jpg", intro_sceneCount);
			lobby->setImage(path);
			intro_sceneCount++;
			if (intro_sceneCount == 34) welcome_ment->play();
			if (intro_sceneCount < 138)
			{
				lobby_timer->set(animationTime);
				lobby_timer->start();
			}
			else
			{
				select_next_button->show();
			}
			
				return 0;
		});


	stage1_scene_timer = Timer::create(animationTime);
	stage1_scene_timer->setOnTimerCallback([&](auto)->bool
		{
			sprintf(path, "Images/무궁화/무궁화 (%d).jpg", stage1_sceneCount);
			stage1_lobby->setImage(path);
			stage1_sceneCount++;
			if (stage1_sceneCount < 123)
				{
				stage1_scene_timer->set(animationTime);
				stage1_scene_timer->start();
				}
			else gameRule_next_button->show();
			return 0;
		});

	stage1_game_timer = Timer::create(game_time_limit);

	stage1_trap_timer = Timer::create(animationTime);
	stage1_trap_timer->setOnTimerCallback([&](auto)->bool
		{
			trapX += 3;
			trap->locate(stage1_gameboard, trapX, trapY);
			trap_Count++;
			if (!check_touch())
			{
				stage1_trap_timer->set(animationTime);
				stage1_trap_timer->start();
			}
			return 0;
		});

	// BGM_main->play();

	game_start_button->setScale(0.4f);
	game_start_button->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)-> bool {

		BGM_main->stop();
		lobby->enter();
		return 0;
		});

	startGame(decide_to_join);
}