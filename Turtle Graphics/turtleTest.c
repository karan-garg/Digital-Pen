#include "turtle.h"

int main()
{
    turtle_init(300, 300);          // initialize the image to be 600x600

    turtle_forward(50);
    turtle_turn_left(90);
    turtle_forward(50);
    turtle_draw_turtle();

    turtle_save_bmp("output.bmp");  // save the turtle drawing

    return 1;
}
