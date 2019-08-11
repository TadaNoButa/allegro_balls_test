#include <iostream>
#include <array>
#include <list>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#define PI 3.14159265

//#define SCREEN_W 640
//#define SCREEN_H 480

#define SCREEN_W 1200
#define SCREEN_H 700


using namespace std;

struct coord2df {
    float x,y;
};

struct coord3df {
    float x,y,z;
};

struct coli {
    int r,g,b;
};

const float FPS = 60;
const float BALLRADIUS = 30.0;
const float SPEEDCOEF = 0.5;
const float DECELCOEF = 0.999;
const float SHOTRATEMAX = 0.1;

const float FRAGRADIUS = 10.0;
const float SPEEDFRAGDOWN = -0.05;
const float FRAGSPEEDCOEFINIT = 0.5;
const float FRAGSPEEDCOEFINITB = 9.0;
const float FRAGSPEEDCOEF = 0.1;
const float FRAGDECELCOEF = 0.95;
const int BALLFRAGDIVV = 5; //was 17
const int BALLFRAGDIVH = 3; //was 9

bool key[ALLEGRO_KEY_MAX];
bool istest = false;
bool isexploded = false;
float timeafteraction = 0.0;

float Deg2Rad (float deg) {
    float rad = (deg/180)*PI;
    return rad;
}

float Rad2Deg (float rad) {
    float deg = (rad/PI)*180;
    return deg;
}

coord2df vector2_add(coord2df v1,coord2df v2) {
  return (coord2df){v1.x+v2.x, v1.y+v2.y};
}

coord2df vector2_minus(coord2df v1,coord2df v2) {
  return (coord2df){v1.x-v2.x, v1.y-v2.y};
}

coord2df vector2_scale(float constant, coord2df v) {
  return (coord2df){constant*v.x, constant*v.y};
}

float vector2_norm(coord2df v) {
  return sqrt(v.x*v.x + v.y*v.y);
}

coord2df vector2_normalize(coord2df v) {
  return vector2_scale(1/vector2_norm(v), v);
}

float vector_dot_product(coord2df a, coord2df b) {
  return a.x*b.x + a.y*b.y;
}


class Player {
    coord2df squaretmp;
  public:
    coord2df pos;
    float timeaftershot;
    Player(float,float);
    void Update();
    bool DoBall();

};

Player::Player(float ix,float iy) {

    pos.x = ix;
    pos.y = iy;
    squaretmp.x = 30.0;
    squaretmp.y = 30.0;

    timeaftershot = 0.0;
}

void Player::Update() {

    //draw
    al_draw_rectangle(this->pos.x-this->squaretmp.x/2,
                    this->pos.y-this->squaretmp.y/2,
                    this->pos.x+this->squaretmp.x/2,
                    this->pos.y+this->squaretmp.y/2,al_map_rgb(0,0,0), 5.0);

    //keyboard
    float quant = 5.0;
    //if(key[ALLEGRO_KEY_DOWN]) this->pos.y += 1;
    //if(key[ALLEGRO_KEY_UP]) this->pos.y -= 1;
    if(key[ALLEGRO_KEY_RIGHT]) this->pos.x += quant;
    if(key[ALLEGRO_KEY_LEFT]) this->pos.x -= quant;

    //timeaftershot
    this->timeaftershot += 1/FPS;

}


bool Player::DoBall() {

    if(key[ALLEGRO_KEY_UP] && this->timeaftershot > SHOTRATEMAX) {
        this->timeaftershot = 0.0;
        return true;
    }
    else return false;

}

class Ball {
    //coord2df pos;
    //coord2df speed;
  public:
    float radius;
    coli col;
    coord2df pos;
    coord2df speed;
    bool collided;
    bool is_active;
    bool is_players;
    static int ind;
    Ball (float,float,int,int,int,float,float,bool);
    void DrawBall();
    void SetSpeed(float,float);
    void MoveBall();
    void CollideWalls();
    void CollideBall(Ball&);
    void AttractBall(Ball&);
};

typedef list<Ball> BALLLISTTYPE;

Ball::Ball(float x, float y, int red, int green, int blue, float spx, float spy, bool isp)
{
    collided = false;
    is_active = true;
    is_players = isp;
    pos.x = x;
    pos.y = y;
    radius = BALLRADIUS;
    col.r = red;
    col.g = green;
    col.b = blue;
    speed.x = spx;
    speed.y = spy;
}

void Ball::DrawBall()
{
    al_draw_filled_circle(this->pos.x, this->pos.y,this->radius, al_map_rgb(this->col.r,this->col.g,this->col.b));
    al_draw_circle(this->pos.x, this->pos.y,this->radius, al_map_rgb(0,0,0),1.0);
}

void Ball::SetSpeed(float spx, float spy)
{
    this->speed.x = spx;
    this->speed.y = spy;
}

void Ball::MoveBall()
{
    this->speed.x = this->speed.x*DECELCOEF;
    this->speed.y = this->speed.y*DECELCOEF;

    this->pos.x += this->speed.x*SPEEDCOEF;
    this->pos.y += this->speed.y*SPEEDCOEF;

}

void Ball::CollideWalls()
{
    //coll left
    if(this->pos.x-this->radius<0.0) {
        this->pos.x = this->radius;
        this->speed.x = -this->speed.x;
    }
    //coll right
    else if(this->pos.x+this->radius>SCREEN_W) {
        this->pos.x = SCREEN_W-this->radius;
        this->speed.x = -this->speed.x;
    }
    //coll up
    if(this->pos.y-this->radius<0.0) {
        this->pos.y = this->radius;
        this->speed.y = -this->speed.y;
    }
    //coll down
    else if(this->pos.y+this->radius>SCREEN_H) {
        this->pos.y = SCREEN_H-this->radius;
        this->speed.y = -this->speed.y;
    }

}

void Ball::CollideBall(Ball& other)
{
  if(!this->collided) {
        if(sqrt(pow((this->pos.x-other.pos.x),2)+pow((this->pos.y-other.pos.y),2))<2*this->radius) {

            //calc center
            coord2df cen = vector2_scale(0.5,vector2_add(this->pos,other.pos));
            cen.x = (this->pos.x+other.pos.x)/2;
            cen.y = (this->pos.y+other.pos.y)/2;
            //calc new pos
            coord2df newposown = vector2_add(cen,vector2_scale(this->radius,vector2_normalize(vector2_minus(this->pos,cen))));
            coord2df newposother = vector2_add(cen,vector2_scale(other.radius,vector2_normalize(vector2_minus(other.pos,cen))));
            //compute new speed
            coord2df dist = vector2_minus(this->pos,other.pos);
            coord2df norm = vector2_normalize(dist);
            float dotprod = vector_dot_product(norm,vector2_minus(this->speed,other.speed));
            coord2df imp = vector2_scale(dotprod,norm);
            this->speed = vector2_minus(this->speed,imp);
            other.speed = vector2_add(other.speed,imp);
            //put new pos
            this->pos = newposown;
            other.pos = newposother;

            /*
            if(this->is_players && !other.is_players) {
                this->col.r = other.col.r;
                this->col.g = other.col.g;
                this->col.b = other.col.b;
            }
            else if(!this->is_players && other.is_players) {
                other.col.r = this->col.r;
                other.col.g = this->col.g;
                other.col.b = this->col.b;
            }
            else if(this->is_players && other.is_players) {

                coli tmp;

                tmp.r = this->col.r;
                tmp.g = this->col.g;
                tmp.b = this->col.b;

                this->col.r = other.col.r;
                this->col.g = other.col.g;
                this->col.b = other.col.b;

                other.col.r = tmp.r;
                other.col.g = tmp.g;
                other.col.b = tmp.b;
            }
            */
        }
    }
}


void Ball::AttractBall(Ball& other)
{
    //
    int spacemult = 10;
    float dist = sqrt(pow((this->pos.x-other.pos.x),2)+pow((this->pos.y-other.pos.y),2));
    if(dist<2*spacemult*this->radius) {

        float forcemult = 10.0;
        this->speed.x += forcemult*(other.pos.x-this->pos.x)/(dist*dist);
        this->speed.y += forcemult*(other.pos.y-this->pos.y)/(dist*dist);

        other.speed.x += forcemult*(this->pos.x-other.pos.x)/(dist*dist);
        other.speed.y += forcemult*(this->pos.y-other.pos.y)/(dist*dist);

    }
}


class Frag {
  public:
    float radius;
    coli col;
    coord3df pos;
    coord3df speed;
    bool is_active;
    Frag (float,float,float,int,int,int,float,float,float);
};

typedef list<Frag> FRAGLISTTYPE;
typedef list<list<Frag>> FRAGLISTLISTTYPE;


Frag::Frag(float x, float y, float z, int red, int green, int blue, float spx, float spy, float spz)
{
    is_active = true;
    pos.x = x;
    pos.y = y;
    pos.z = z;
    radius = FRAGRADIUS;
    col.r = red;
    col.g = green;
    col.b = blue;
    speed.x = spx;
    speed.y = spy;
    speed.z = spz;
}



void Draw(BALLLISTTYPE& balllist)
{
    for (BALLLISTTYPE::iterator it = balllist.begin(); it != balllist.end(); ++it) it->DrawBall();
}

void Move(BALLLISTTYPE& balllist)
{
    //cout << "move size:" << balllist.size() <<'\n';

    for (BALLLISTTYPE::iterator it = balllist.begin(); it != balllist.end(); ++it) {
        it->MoveBall();
    }
    //cout << "move end" <<'\n';
}

void Attract(BALLLISTTYPE& balllist)
{

    //btw balls
    for (BALLLISTTYPE::iterator it1 = balllist.begin(); it1 != balllist.end(); ++it1) {

        BALLLISTTYPE::iterator it2 = balllist.end();
        for (it2--; it2 != balllist.begin(); --it2) {

            if(it1==it2) break;
            it1->AttractBall(*(it2));
        }
    }
}



void Collide(BALLLISTTYPE& balllist)
{
    //walls(
    for (BALLLISTTYPE::iterator it = balllist.begin(); it != balllist.end(); ++it) it->CollideWalls();

    //btw balls
    for (BALLLISTTYPE::iterator it1 = balllist.begin(); it1 != balllist.end(); ++it1) {

        BALLLISTTYPE::iterator it2 = balllist.end();
        for (it2--; it2 != balllist.begin(); --it2) {

            if(it1==it2) break;
            it1->CollideBall(*(it2));
        }
    }
}


void Initballs(BALLLISTTYPE& balllist)
{

    balllist.push_back(Ball(250.0,400.0,255,0,0,0.0,-10.0,false));
    balllist.push_back(Ball(230.0,200.0,0,255,0,5.0,5.0,false));
    balllist.push_back(Ball(50.0,150.0,0,0,255,1.0,-15.0,false));

}

void Explodeballs(BALLLISTTYPE& balllist, FRAGLISTLISTTYPE& flistlist)
{

    timeafteraction += 1/FPS;

    if(key[ALLEGRO_KEY_SPACE] && timeafteraction > 0.5) {

        if(isexploded == false) {

            cout << "expl=false before" << '\n';
            cout << "listlist size: " << flistlist.size() << '\n';
            cout << "balllist size: " << balllist.size() << '\n';

            float vq = 360/BALLFRAGDIVV;
            float hq = 360/BALLFRAGDIVH;

            int i = 0;
            //for (BALLLISTTYPE::iterator it = balllist.begin(); it != balllist.end(); ++it)
            while(!balllist.empty())
            {
                Ball it = balllist.front();

                cout << "i: " << i << '\n';

                FRAGLISTTYPE fraglist;
                for(int iv=0; iv<BALLFRAGDIVV;iv++) {

                    /*
                    float zdec = sin(Deg2Rad(iv*vq))*it->radius;
                    float rprime = cos(Deg2Rad(iv*vq))*it->radius;

                    for(int ih=0; ih<BALLFRAGDIVH;ih++) {

                        float xdec = cos(Deg2Rad(ih*hq))*rprime;
                        float ydec = sin(Deg2Rad(ih*hq))*rprime;
                        fraglist.push_back(Frag(it->pos.x+xdec,it->pos.y+ydec,it->radius+zdec,it->col.r,it->col.g,it->col.b,
                        it->speed.x*FRAGSPEEDCOEFINITB+xdec*FRAGSPEEDCOEFINIT,it->speed.y*FRAGSPEEDCOEFINITB+ydec*FRAGSPEEDCOEFINIT,SPEEDFRAGDOWN+zdec));
                    }
                    */

                    float zdec = sin(Deg2Rad(iv*vq))*it.radius;
                    float rprime = cos(Deg2Rad(iv*vq))*it.radius;

                    for(int ih=0; ih<BALLFRAGDIVH;ih++) {

                        float xdec = cos(Deg2Rad(ih*hq))*rprime;
                        float ydec = sin(Deg2Rad(ih*hq))*rprime;
                        fraglist.push_back(Frag(it.pos.x+xdec,it.pos.y+ydec,it.radius+zdec,it.col.r,it.col.g,it.col.b,
                        it.speed.x*FRAGSPEEDCOEFINITB+xdec*FRAGSPEEDCOEFINIT,it.speed.y*FRAGSPEEDCOEFINITB+ydec*FRAGSPEEDCOEFINIT,SPEEDFRAGDOWN+zdec));
                    }

                }
                flistlist.push_back(fraglist);

                //it = balllist.erase(it);
                balllist.pop_front();

                i++;
            }
            istest = true;
            isexploded = true;

            cout << "expl=false after" << '\n';
            cout << "listlist size: " << flistlist.size() << '\n';
            cout << "balllist size: " << balllist.size() << '\n';
        }
        else {

            cout << "expl=true before" << '\n';
            cout << "listlist size: " << flistlist.size() << '\n';
            cout << "balllist size: " << balllist.size() << '\n';


            while(!flistlist.empty()) flistlist.pop_front();

            Initballs(balllist);

            isexploded = false;


            cout << "expl=true after" << '\n';
            cout << "listlist size: " << flistlist.size() << '\n';
            cout << "balllist size: " << balllist.size() << '\n';

        }
        timeafteraction = 0.0;
    }
}

void Fraginfos(FRAGLISTLISTTYPE& flistlist)
{
    cout << "listlist size: " << flistlist.size() << '\n';
    for (FRAGLISTLISTTYPE::iterator itlist = flistlist.begin(); itlist != flistlist.end(); ++itlist)
    {

        cout << "one list" << '\n';
        for (FRAGLISTTYPE::iterator it = itlist->begin(); it != itlist->end(); ++it)
        {

            cout << "f ";

        }
        cout << '\n';

    }
    istest = false;
}


void UpdateFrags(FRAGLISTLISTTYPE& flistlist)
{
    for (FRAGLISTLISTTYPE::iterator itlist = flistlist.begin(); itlist != flistlist.end(); ++itlist)
    {
        //cout << "one list" << '\n';
        for (FRAGLISTTYPE::iterator it = itlist->begin(); it != itlist->end(); ++it)
        {
            //move
            it->speed.x = it->speed.x*FRAGDECELCOEF;
            it->speed.y = it->speed.y*FRAGDECELCOEF;
            it->speed.z = it->speed.z*FRAGDECELCOEF;

            it->pos.x += it->speed.x*FRAGSPEEDCOEF;
            it->pos.y += it->speed.y*FRAGSPEEDCOEF;
            it->pos.z += it->speed.z*FRAGSPEEDCOEF;

            //draw
            al_draw_filled_circle(it->pos.x, it->pos.y,it->radius, al_map_rgb(it->col.r,it->col.g,it->col.b));
            al_draw_circle(it->pos.x, it->pos.y,it->radius, al_map_rgb(0,0,0),1.0);
        }
    }

}


int main (int argc, char **argv) {


    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *alqueue = NULL;
    ALLEGRO_EVENT event;
    ALLEGRO_TIMER *timer = NULL;

    al_init();
    al_init_primitives_addon();

    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
        cout << "failed to create timer!" << '\n';
        return -1;
    }

    display = al_create_display(SCREEN_W, SCREEN_H);
    if(!display) {
        cout << "failed to create display!" << '\n';
        return -1;
    }


    alqueue = al_create_event_queue();
    if(!alqueue) {
        cout << "failed to create event_queue!" << '\n';
        return -1;
    }

    al_install_keyboard();
    al_install_joystick();


//    joy = al_get_joystick(0);

    al_register_event_source(alqueue,al_get_keyboard_event_source());
    al_register_event_source(alqueue, al_get_display_event_source(display));
    al_register_event_source(alqueue, al_get_timer_event_source(timer));

    al_register_event_source(alqueue,al_get_joystick_event_source());
    al_start_timer(timer);


    ALLEGRO_COLOR cwhite = al_map_rgb(255,255,255);

    Player player {SCREEN_W/2,SCREEN_H-30.0};


    BALLLISTTYPE balllist;
    FRAGLISTLISTTYPE flistlist;

    Initballs(balllist);

    while(true) {

        al_wait_for_event(alqueue, &event);

        if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) break;

        else if(event.type == ALLEGRO_EVENT_KEY_DOWN) {

            if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) break;
            key[event.keyboard.keycode] = true;

        }
        else if(event.type == ALLEGRO_EVENT_KEY_UP) key[event.keyboard.keycode] = false;

        else if(event.type == ALLEGRO_EVENT_TIMER) {

            if(al_is_event_queue_empty(alqueue)) {


                al_clear_to_color(cwhite);

                //player.timeaftershot =+ 1/FPS;


                //cout << "player.timeaftershot:" << player.timeaftershot <<'\n';

                player.Update();
                if(player.DoBall()) balllist.push_back(Ball(player.pos.x,player.pos.y,0,0,0,0.0,-10.0,true));

                Draw(balllist);
                Attract(balllist);
                Collide(balllist);
                Move(balllist);
                Explodeballs(balllist,flistlist);
                //if(istest) Fraginfos(flistlist);
                UpdateFrags(flistlist);

                al_flip_display();
            }

        }
    }

    return 0;
}
