#include "my_headers.h"

/********************************/
/*Code added for sockets*/
#include "sockets/socket.h"
#include <process.h>

#define SERVER_PORT 10000
#define CLIENT_PORT 2001
#define BUFFER_SIZE 100
/*******************************/
/*continuing with sockets*/
typedef std::list<Socket*> socket_list;

socket_list g_connections;

unsigned __stdcall Connection(void* a) {
	Socket* s = (Socket*) a;

	g_connections.push_back(s);

	s->SendLine("Welcome to the Message Distributor");
	std::string r = s->ReceiveLine();
    if (!r.empty())
		for (socket_list::iterator os =g_connections.begin();os!=g_connections.end();os++)
			if (*os != s) 
				(*os)->SendLine(r);

	g_connections.remove(s);

	delete s;

	return 0;
}





unsigned char sticks=0;
unsigned char dticks=0;
unsigned char SecondTick=0;
unsigned char DisplayTick=0;
int FPS=0;
int iFPS=0;
int CPS=0;
int iCPS=0;

void Seconds_Tick(void)
{
    if (sticks==255) {sticks=0;SecondTick=0;}
    sticks++;
}
END_OF_FUNCTION(Seconds_Tick);

void Display_Tick(void)
{
    if (dticks==255) {dticks=0;DisplayTick=0;}
    dticks++;
}
END_OF_FUNCTION(Display_Tick);

/*/////////////////////////////////////////////////////////////////////////////////////////////
	//									MAIN									//
////////////////////////////////////////////////////////////////////////////////////////////*/
int main()
{
 
    allegro_init();
    install_timer();
    install_keyboard();
    install_timer();

	LOCK_VARIABLE(dticks);
    LOCK_FUNCTION(Display_Tick);
    LOCK_VARIABLE(sticks);
    LOCK_FUNCTION(Seconds_Tick);
	install_int_ex(&Display_Tick, MSEC_TO_TIMER(1));
    install_int_ex(&Seconds_Tick, SECS_TO_TIMER(1));

	set_color_depth(16);
    set_gfx_mode( GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);
	BITMAP* buffer,*buffer_2;
	buffer_2 =  create_bitmap(SCREEN_W,SCREEN_H);

	/******************!!!!!!!!!!!!!!!remove the comments for the welcome screen!!!!!!!!!!!!************************************/
	//Rendering::Singleton().DrawWelcomeScreen();/*Drawing the welcome screen*/
	/* MISSING -> Menu for players drawing     */
	/* MISSING -> Taking the choice of players */
	unsigned short player_num = 2;
	GameInitialization::Singleton();
	GameInitialization::Singleton().SetPlayers(player_num);
	GameInitialization::Singleton().InitAll();
	Game::Singleton();
	Game::Singleton().SetPlayers(player_num);
	PacMan* pac_sprite = GameInitialization::Singleton().GetPacman();
	Game::Singleton().CheckMarksCollision();

	/*INITIAL RENDERING*/
	Rendering::Singleton().DrawObstacles();
	Rendering::Singleton().DrawPowerMarks();
	Rendering::Singleton().DrawSimpleMarks();
	Rendering::Singleton().DrawLives();

	buffer = Rendering::Singleton().GetBuffer();
	blit(buffer,screen,0,0,0,0,SCREEN_W,SCREEN_H);
	//Rendering::Singleton().DrawReadyScreen();
	///BITMAP* fps_b  = create_bitmap(100,10);/*Bitmap that displays the fps of the game*/
	blit(screen,buffer,0,0,0,0,SCREEN_W,SCREEN_H);
	/*END OF INITIAL RENDERING*/
	bool server;
	FILE* fp;
	while(!key[KEY_W])
	{
		if(key[KEY_1])
		{
			server = true;
			textprintf_ex(screen, font, 0, 10, makecol(255, 255, 255), -1, "server");
			fp = fopen("lol.txt","w");
		}
		if(key[KEY_2])
		{
			server = false;
			textprintf_ex(screen, font, 0, 10, makecol(255, 255, 255), -1, "client");
			fp = fopen("lol1.txt","w");
		}
	}
	/*Socket initialization*/	
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,0),&wsaData) != 0)
    {
		perror("Initialization error");fprintf(fp,"Initialization error");
		exit(1);
	}

	while( !key[KEY_ESC] && !Game::Singleton().IsGameOver() && !Game::Singleton().IsGameCompleted())
	{
		if(server == true)/*server case*/
		{
			char     dir[BUFFER_SIZE];  /* used for incomming dir name, and outgoing data */
			int 	 server_socket, sd_current;
			int 	 addrlen;
			struct   sockaddr_in server_address;
			struct   sockaddr_in pin;
 
			/* get an internet domain socket */
			if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
				perror("socket");fprintf(fp,"server socket %d",WSAGetLastError());
				exit(1);
			}
			
			/* complete the socket structure */
			memset(&server_address, 0, sizeof(server_address));
			server_address.sin_family = AF_INET;
			server_address.sin_addr.s_addr = INADDR_ANY;
			server_address.sin_port = htons(SERVER_PORT);
			memset(server_address.sin_zero,0,8);

			textprintf_ex(screen, font, 0, 20, makecol(255, 255, 255), -1, "Hi there, i am  %s",inet_ntoa(server_address.sin_addr));

			/* bind the socket to the port number */
			if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == SOCKET_ERROR) {
				perror("bind");fprintf(fp,"server bind %d",WSAGetLastError());
				exit(1);
			}

			/* show that we are willing to listen */
			if (listen(server_socket, 10) == -1) {
				perror("listen");fprintf(fp,"server listen");
				exit(1);
			}
			/* wait for a client to talk to us */
			addrlen = sizeof(pin); 
			if ((sd_current = accept(server_socket, (struct sockaddr *)  &pin, &addrlen)) == -1) {
				perror("accept");fprintf(fp,"server accept");
				exit(1);
			}
			/* if you want to see the ip address and port of the client, uncomment the 
			next two lines */

			/*
			printf("Hi there, from  %s#\n",inet_ntoa(pin.sin_addr));
			printf("Coming from port %d\n",ntohs(pin.sin_port));
			*/

			/* get a message from the client */
			if (recv(sd_current, dir, sizeof(dir), 0) == -1) {
				perror("recv");fprintf(fp,"server recv");
				exit(1);
			}
			textprintf_ex(screen, font, 0, 30, makecol(255, 255, 255), -1, "I recieved %s",dir);
			/* get the directory contents */
			//read_dir(dir);
			memset(dir,'\0',100);
			strcpy(dir,"ACK DUDE");
      
			/* acknowledge the message, reply w/ the file names */
			if (send(sd_current, dir, strlen(dir), 0) == -1) {
			perror("send");fprintf(fp,"server send");
			exit(1);
			}

			/* close up both sockets */
			closesocket(sd_current); closesocket(server_socket);
		}
		else/*client case*/
		{
			char hostname[100];
			char    dir[BUFFER_SIZE];
			int	client_socket;
			//struct sockaddr_in sin;
			struct sockaddr_in server_address;
			struct hostent *hp;

		    //strcpy(hostname,"GeorgeK");
			strcpy(hostname,"localhost");

			/* go find out about the desired host machine */
			if ((hp = gethostbyname(hostname)) == 0) {
			perror("gethostbyname");fprintf(fp,"hosting");
			exit(1);
			}

			/* fill in the socket structure with host information */
			memset(&server_address, 0, sizeof(server_address));
			server_address.sin_family = AF_INET;
			server_address.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
			//server_address.sin_addr.s_addr = inet_addr(hostname);
			server_address.sin_port = htons(SERVER_PORT);
			memset(server_address.sin_zero,0,8);

char buf[100];
gethostname(buf,100);
fprintf(fp,"client what? %s\n",buf);

			textprintf_ex(screen, font, 0, 20, makecol(255, 255, 255), -1, 
				"Hi there, i connect  %s",inet_ntoa(server_address.sin_addr));

			//while(!key[KEY_L])
			//{}
			/* grab an Internet domain socket */
			if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
				perror("socket");fprintf(fp,"socket %d\n",WSAGetLastError());
				exit(1);
			}

			/* connect to PORT on HOST */
			if (connect(client_socket,(struct sockaddr *)  &server_address, sizeof(server_address)) == -1) {
				perror("connect");fprintf(fp,"connect %d",WSAGetLastError());
				exit(1);
			}

fprintf(fp,"passed connect???\n");
			/* send a message to the server PORT on machine HOST */
			if (send(client_socket, "one message by the client", strlen("one message by the client"), 0) == -1) {
				perror("send");fprintf(fp,"send %d",WSAGetLastError());
				exit(1);
			}

			/* wait for a message to come back from the server */
			if (recv(client_socket, dir, BUFFER_SIZE, 0) == -1) {
                perror("recv");fprintf(fp,"recv %d",WSAGetLastError());
                exit(1);
			}

			/* spew-out the results and bail out of here! */
			printf("%s\n", dir);
			
			closesocket(client_socket);
		}

		if(sticks - SecondTick > 0)
        {
			/*Game FPS Counter*/
            FPS=iFPS;
            iFPS=0;
            CPS=iCPS;
            iCPS=0;
            SecondTick=sticks;
        }
		 if (dticks - DisplayTick > 0)
        {
			if (key[KEY_P])
				Game::Singleton().Pause();
			/*MOVEMENT*/
			Game::Singleton().SetGameTime();
			Game::Singleton().PacmanMove();
			Game::Singleton().GhostMove();
			Game::Singleton().ObstacleMoveAll();/*Movement for obstacles*/
			AnimatorHolder::Progress(Game::Singleton().GetGameTime());
			
			/*COLLISION CHECK*/
			CollisionChecker::Singleton().Check();
			DestructionManager::Commit();
			
			if(pac_sprite->CanEat())
				Game::Singleton().CheckBonusTimer();
			
			/*RENDERING*/
			//Rendering::Singleton().EraseAll();
			clear_to_color(Rendering::Singleton().GetBuffer(),makecol(0,0,0));

			Rendering::Singleton().DrawObstacles();
			Rendering::Singleton().DrawLives();
			Rendering::Singleton().DrawPowerMarks();
			Rendering::Singleton().DrawSimpleMarks();
			Rendering::Singleton().DrawFPS(FPS);
			Rendering::Singleton().DrawAll();
			Rendering::Singleton().DrawScore();

			buffer = Rendering::Singleton().GetBuffer();
			blit(buffer,screen,0,0,0,0,SCREEN_W,SCREEN_H);

			/*Check the game if it is time to draw*/
			Game::Singleton().CheckGameStatus();
			if(!Game::Singleton().IsGameOver() && CollisionChecker::Singleton().GetLostLife())
			{
				Rendering::Singleton().DrawReadyScreen();
				CollisionChecker::Singleton().SetLostLife(false);
			}
			/*End of game status drawing*/
			
			clear_keybuf();
			
			DisplayTick=dticks;
		    iFPS++;
		}
		 iCPS++;
	}

	if(Game::Singleton().IsGameOver())
		Rendering::Singleton().DrawGameOver();
	if(Game::Singleton().IsGameCompleted())
		Rendering::Singleton().DrawGameCompleted();
	while(!key[KEY_ESC]){}
	allegro_exit();
    return 0;
}   
END_OF_MAIN(); 




		//clear_to_color(b,makecol(0,0,0));
		//textprintf_ex(b, font, 0, 0, makecol(255, 255, 255), -1, "%d",Game::Singleton().GetMarks());
		//blit(b,screen,0,0,0,20,50,50);









//	for(i = sml.begin();i!=sml.end();i++)
//	{
//		int x = (*i)->GetX() - pac_sprite->GetX();
//		int y = (*i)->GetX() - pac_sprite->GetY();
//		if( x > 0 && x < 30 && y > 0 && y < 30)
//		{
//			CollisionChecker::Singleton().Register(pac_sprite , *i);n++;
//		}
	//	else
	//			CollisionChecker::Singleton().Cancel(pac_sprite , *i);
		//textprintf_ex(screen, font, 10, 450, makecol(255, 255, 255), -1, "found %d",n);
//	}
//	for(p = pml.begin();p!=pml.end();p++)
//	{
//		
//		int x = (*p)->GetX() - pac_sprite->GetX();
//		int y = (*p)->GetX() - pac_sprite->GetY();
//		if( x > 0 && x < 30 && y > 0 && y < 30)
//		{
//			CollisionChecker::Singleton().Register(pac_sprite , *p);
//			n++;
//		}
	//	else
	//		CollisionChecker::Singleton().Cancel(pac_sprite , *p);
		//textprintf_ex(screen, font, 10, 450, makecol(255, 255, 255), -1, "found %d",n);
//	}