#include <stdlib.h>
// #include <stdio.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

typedef struct {
    Vector2 pos;
    Vector2 vel;
    Vector2 movedir;
    float rot;
    float spd;
    float drag;
} Player;

float castRayDDA
(
    Vector2 start_pos,
    Vector2 direction,
    int** map,
    int map_rows,
    int map_cols,
    float tile_size,
    float max_distance
);

void updateTiles
(
    int** map,
    int map_rows,
    int map_cols,
    float tile_size
);
void updatePlayer(Player* p, float* mouse_sensitivity);

void draw2D
(
    float screen_width,
    float screen_height,
    Player p,
    int** map,
    int map_rows,
    int map_cols,
    float tile_size
);
void draw3D
(
    float screen_width,
    float screen_height,
    Player p,
    int** map,
    int map_rows,
    int map_cols,
    float tile_size,
    float max_ray_distance
);

int main(void) {
    const float screen_width = 800.0f;
    const float screen_height = 800.0f;

    InitWindow((int)screen_width, (int)screen_height, "raycasting renderer");

    // INITIALIZE
    Player p = {
        .pos = (Vector2){ screen_width / 2.0f, screen_height / 2.0f },
        .vel = (Vector2){0},
        .movedir = (Vector2){0},
        .rot = 0.0f,
        .spd = 0.5f,
        .drag = 0.14f,
    };

    float mouse_sensitivity = 0.005f;

    const int map_rows = 20;
    const int map_cols = 20;

    int** map = malloc(sizeof (int*) * map_rows);
    for (int i = 0; i < map_rows; i++) {
        map[i] = calloc(map_cols, sizeof (int));
    }

    const float tile_size = 40.0f;

    const int font_size = 20;
    const int margin = 5;

    enum { MODE_2D, MODE_3D } mode = MODE_2D;

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        // UPDATE
        // toggle the display mode between 2D and 3D
        if (IsKeyPressed(KEY_TAB)) {
            if (mode == MODE_2D) {
                mode = MODE_3D;
            } else {
                mode = MODE_2D;
            }
        }
        
        switch (mode) {
            case MODE_2D:
            {
                updateTiles(map, map_rows, map_cols, tile_size);
            } break;
            case MODE_3D:
            {
                updatePlayer(&p, &mouse_sensitivity);
            } break;
        }

        BeginDrawing();
        // DRAW
        ClearBackground(BLACK);
        switch (mode) {
            case MODE_2D:
            {
                // show the cursor to enable editing the tiles in top down mode
                ShowCursor();
                draw2D(screen_width, screen_height, p, map, map_rows, map_cols, tile_size);
                DrawRectangle(
                    0,
                    0,
                    295,
                    5 + 4 * font_size + 4 * margin,
                    BLACK
                );
                DrawText(
                    "[left click] to paint walls",
                    5,
                    5 + font_size + margin,
                    font_size,
                    WHITE
                );
                DrawText(
                    "[right click] to remove walls",
                    5,
                    5 + 2 * font_size + 2 * margin,
                    font_size,
                    WHITE
                );
                DrawText(
                    "[c] to remove all walls",
                    5,
                    5 + 3 * font_size + 3 * margin,
                    font_size,
                    WHITE
                );
            } break;
            case MODE_3D:
            {
                // hide the cursor in first person mode
                HideCursor();
                draw3D(
                    screen_width,
                    screen_height,
                    p,
                    map,
                    map_rows,
                    map_cols,
                    tile_size,
                    5000.0f
                );
                DrawRectangle(
                    0,
                    0,
                    280,
                    5 + 4 * font_size + 4 * margin,
                    BLACK
                );
                DrawText(
                    "[wasd] to move",
                    5,
                    5 + font_size + margin,
                    font_size,
                    WHITE
                );
                DrawText(
                    "[mouse] to look",
                    5,
                    5 + 2 * font_size + 2 * margin,
                    font_size,
                    WHITE
                );
                DrawText(
                    "[j/k] to change sensitivity",
                    5,
                    5 + 3 * font_size + 3 * margin,
                    font_size,
                    WHITE
                );
            } break;
        }
        DrawText("[tab] to switch mode", 5, 5, font_size, WHITE);
        EndDrawing();
    }
    
    // UNINITIALIZE
    for (int i = 0; i < map_rows; i++) {
        free(map[i]);
    }
    free(map);
    CloseWindow();
    return EXIT_SUCCESS;
}

// updates the tiles based on user input
void updateTiles
(
    int** map,
    int map_rows,
    int map_cols,
    float tile_size
) {
    Vector2 mouse_pos = GetMousePosition();

    // get the x and y coordinates of the mouse in grid space
    // to know which cell it is hovering over
    int mouse_cell_x = (int)(mouse_pos.x / tile_size);
    int mouse_cell_y = (int)(mouse_pos.y / tile_size);

    // make sure the mouse is not outside of the grid to avoid
    // accessing invalid memory
    if (
        mouse_cell_x >= 0 && mouse_cell_x < map_cols &&
        mouse_cell_y >= 0 && mouse_cell_y < map_rows
    ) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            // set the tile the mouse is over to be a wall (1)
            map[mouse_cell_y][mouse_cell_x] = 1;
        } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            // set the tile the mouse is over to be empty (0)
            map[mouse_cell_y][mouse_cell_x] = 0;
        }
    }

    if (IsKeyPressed(KEY_C)) {
        // clear the whole grid
        for (int i = 0; i < map_rows; i++) {
            for (int j = 0; j < map_cols; j++) {
                map[i][j] = 0;
            }
        }
    }
}

// updates the player based on user input
void updatePlayer(Player* p, float* mouse_sensitivity) {
    // update the mouse sensitivity
    if (IsKeyPressed(KEY_J)) {
        *mouse_sensitivity += 0.001f;
    } else if (IsKeyPressed(KEY_K)) {
        *mouse_sensitivity -= 0.001f;
    }

    // clamp the mouse sensitivity
    if (*mouse_sensitivity < 0.0001f) {
        *mouse_sensitivity = 0.0001f;
    } else if (*mouse_sensitivity > 0.05f) {
        *mouse_sensitivity = 0.05f;
    }
    
    // update rotation on the basis of mouse movement on the x-axis
    // multiply by mouse sensitivity to scale the amount
    p->rot += GetMouseDelta().x * *mouse_sensitivity;

    // set the mouse position to the middle of the screen to avoid
    // it reaching the actual ends of the monitor and not updating the
    // mouse delta correctly
    SetMousePosition(400, 400);
    
    // movement direction accumulator to enable strafing
    Vector2 movedir_acc = {0};
    
    // update move direction for forward/backward movement
    if (IsKeyDown(KEY_W)) {
        movedir_acc.x += cosf(p->rot);
        movedir_acc.y += sinf(p->rot);
    } else if (IsKeyDown(KEY_S)) {
        movedir_acc.x -= cosf(p->rot);
        movedir_acc.y -= sinf(p->rot);
    }

    // update move direction for strafing
    if (IsKeyDown(KEY_A)) {
        movedir_acc.x += sinf(p->rot);
        movedir_acc.y -= cosf(p->rot);
    } else if (IsKeyDown(KEY_D)) {
        movedir_acc.x -= sinf(p->rot);
        movedir_acc.y += cosf(p->rot);
    }

    // set the player's movement direction to the direction calculated above
    p->movedir = movedir_acc;

    // add to the velocity in the direction the player moved
    p->vel = Vector2Add(p->vel, Vector2Scale(p->movedir, p->spd));

    // apply drag/friction to player's velocity
    p->vel = Vector2Subtract(p->vel, Vector2Scale(p->vel, p->drag));

    // update the player position based on the velocity
    p->pos = Vector2Add(p->pos, p->vel);

    // update view direction
    // p->viewdir = (Vector2){ cosf(p->rot), sinf(p->rot) };
}

// draws the world in 2D
void draw2D
(
    float screen_width,
    float screen_height,
    Player p,
    int** map,
    int map_rows,
    int map_cols,
    float tile_size
) {

    // draw horizontal grid lines
    for (int i = 0; i < map_rows; i++) {
        DrawLine(0, i * tile_size, screen_width, i * tile_size, GRAY);
    }
    // draw vertical grid lines
    for (int i = 0; i < map_cols; i++) {
        DrawLine(i * tile_size, 0, i * tile_size, screen_height, GRAY);
    }

    // draw map tiles
    for (int i = 0; i < map_rows; i++) {
        for (int j = 0; j < map_cols; j++) {
            if (map[i][j] == 1)
            DrawRectangle(
                j * tile_size,
                i * tile_size,
                tile_size,
                tile_size,
                WHITE
            );
        }
    }

    // draw player
    DrawCircleV(p.pos, 10.0f, RED);
    // draw player's view direction
    const Vector2 viewdir = { cosf(p.rot), sinf(p.rot) };
    DrawLineV(p.pos, Vector2Add(p.pos, Vector2Scale(viewdir, 20.0f)), GREEN);
}

// draws the world in 3D
void draw3D
(
    float screen_width,
    float screen_height,
    Player p,
    int** map,
    int map_rows,
    int map_cols,
    float tile_size,
    float max_ray_distance
) {
    // color ceiling
    DrawRectangle(
        0,
        0,
        screen_width,
        (int)(screen_height / 2.0f),
        GRAY
    );
    // color floor
    DrawRectangle(
        0,
        (int)(screen_height / 2.0f),
        screen_width,
        (int)(screen_height / 2.0f),
        LIGHTGRAY
    );
    
    // get the starting angle and ending angle for the raycaster based on the field
    // of view
    const float fov = 60.0f;
    const float start_rot = p.rot - (DEG2RAD * (fov / 2.0f));
    const float end_rot = p.rot + (DEG2RAD * (fov / 2.0f));

    // calculate the step size needed to draw one column per pixel on the screen
    const float full_rot_range = end_rot - start_rot;
    const float rad_per_px = full_rot_range / screen_width;

    // the current column in pixels
    int i = 0;
    // the current rotation for the ray in radians
    float rot = start_rot;
    // loop through every pixel column on the screen
    // this makes the x-axis essentially software rendered
    while (rot <= end_rot) {
        // get the direction vector based on the angle of the current ray
        const Vector2 rot_dir = { cosf(rot), sinf(rot) };

        // get the distance to the next wall the ray hits
        float dist_to_wall = castRayDDA(
            p.pos,
            rot_dir,
            map,
            map_rows,
            map_cols,
            tile_size,
            max_ray_distance
        );

        // go to the next iteration without drawing anything if the ray has not
        // hit a wall
        if (dist_to_wall == max_ray_distance) {
            i += 1;
            rot += rad_per_px;
            continue;
        };

        // correct fish eye effect
        dist_to_wall *= cosf(rot - p.rot);

        // get the correct display height for the current slice of the wall
        const float wall_height = (tile_size * screen_height) / dist_to_wall;

        // get the y coordinate for the top of the current slice of the wall
        int wall_top_y = (int)((screen_height / 2.0f) - (wall_height / 2.0f));
        wall_top_y = (wall_top_y < 0) ? 0 : wall_top_y; 

        // get the y coordinate for the bottom of the current slice of the wall
        int wall_bottom_y = (int)((screen_height / 2.0f) + (wall_height / 2.0f));
        wall_bottom_y = (wall_bottom_y > (int)screen_height) ? (int)screen_height : wall_bottom_y;

        // draw the current slice of the wall
        // using rectangle to utilize the GPU for better performance
        DrawRectangle(
            i,
            wall_top_y,
            1,
            wall_bottom_y - wall_top_y,
            WHITE
        );

        // increment the values used to cast the ray and draw the wall slice
        i += 1;
        rot += rad_per_px;
    }
    DrawFPS((int)screen_width - 30, 5);
}

// returns the distance a ray needs to travel to hit a wall on a 2D grid
// if it doesn't hit a wall, returns the maximum distance the ray is allowed to travel
float castRayDDA
(
    // the starting position of the ray (x/y coordinates)
    Vector2 start_pos,
    // the direction the ray is cast in (normalized unit vector)
    Vector2 direction,
    // a 2D grid of cells that can be marked as wall
    // NOTE: the pointer type may differ depending on how the map is structured
    int** map,
    // the number of rows in the grid
    int map_rows,
    // the number of columns in the grid
    int map_cols,
    // the side length of one grid cell
    float tile_size,
    // the maximum distance the ray is allowed to travel
    float max_distance
) {
    // calculate the direction to step in pixel space each iteration relative to one
    // grid cell
    // these are constant because the ray follows a linear line which means that the
    // directions to move in never change
    // multiplying the values with the tile_size gives the distance the ray needs to
    // travel to cross one grid cell
    const Vector2 step_dir = {
        .x = sqrtf(1.0f + (direction.y / direction.x) * (direction.y / direction.x)),
        .y = sqrtf(1.0f + (direction.x / direction.y) * (direction.x / direction.y)),
    };

    // calculate the initial grid space coordinates the ray starts from
    // they will be incremented and decremented during iteration to indicate the
    // coordinates of the cell the ray is currently in
    // these are used to index the map array to check if the current cell is a wall
    int cur_map_x = (int)(start_pos.x / tile_size);
    int cur_map_y = (int)(start_pos.y / tile_size);

    // calculate the direction to step in in grid space each iteration
    // these are constant because the ray follows a linear line which means that the
    // directions to move in never change
    // NOTE: to improve performance, this may be done together with calculating the
    //       initial ray_len to reduce the number of individual checks from 4 to 2
    //       step_x and step_y cannot be const in this case
    const int step_x = (direction.x < 0.0f) ? -1 : 1;
    const int step_y = (direction.y < 0.0f) ? -1 : 1;

    // calculate the initial lengths that the ray needs to travel in each direction to
    // hit the first grid line
    // NOTE: if step_x, step_y and ray_len are calculated in the same step, the if
    //       conditions need to look like this:
    //           if (direction.x < 0.0f) {...} else {...}
    //           if (direction.y < 0.0f) {...} else {...}
    //       initialize both step_x and ray_len.x as well as step_y and ray_len.y
    //       respectively if the corresponding conditions are met
    Vector2 ray_len;
    if (step_x == -1) {
        ray_len.x = (start_pos.x - (float)cur_map_x * tile_size) * step_dir.x;
    } else {
        ray_len.x = ((float)(cur_map_x + 1) * tile_size - start_pos.x) * step_dir.x;
    }
    if (step_y == -1) {
        ray_len.y = (start_pos.y - (float)cur_map_y * tile_size) * step_dir.y;
    } else {
        ray_len.y = ((float)(cur_map_y + 1) * tile_size - start_pos.y) * step_dir.y;
    }
    
    // denotes if the ray has hit a wall during travel through the grid
    // used to break out of the loop that calculates the total distance traveled
    bool has_hit_wall = false;

    // is set to the larger value of ray_len.x or ray_len.y each iteration to simplify
    // checks and calculating the end_pos as well as to not overstep if the ray hits a
    // wall
    float distance = 0.0f;

    while (!has_hit_wall && distance < max_distance) {
        // check if the next grid line that the ray hits is horizontal or vertical
        if (ray_len.x < ray_len.y) {
            // step in the horizontal direction in grid space
            cur_map_x += step_x;

            // cache the distance to not overstep if the ray hits a wall
            distance = ray_len.x;

            // accumulate the distance traveled on the x-axis in pixel space
            // the step_dir is multiplied by the tile_size to get the length in
            // pixel space
            ray_len.x += step_dir.x * tile_size;
        } else {
            // step in the vertical direction in grid space
            cur_map_y += step_y;

            // cache the distance to not overstep if the ray hits a wall
            distance = ray_len.y;

            // accumulate the distance traveled on the y-axis in pixel space
            // the step_dir is multiplied by the tile_size to get the length in
            // pixel space
            ray_len.y += step_dir.y * tile_size;
        }

        // check the array bounds to avoid accessing invalid memory
        if
        (
            cur_map_x >= 0 && cur_map_x < map_cols &&
            cur_map_y >= 0 && cur_map_y < map_rows
        ) {
            // check if the grid cell at the current ray location in grid space
            // is marked as a wall
            // NOTE: depending on the type of the values stored in the array,
            //       this check may access fields or use different logic to
            //       determine if a grid cell is marked as a wall
            if (map[cur_map_y][cur_map_x] == 1) {
                // set has_hit_wall to true to break out of the loop on the next
                // iteration and retain the correct value for distance
                has_hit_wall = true;
            }
        }
    }

    // return the distance the ray has traveled if it has hit a wall,
    // otherwise return the maximum distance the ray is allowed to travel
    return (has_hit_wall) ? distance : max_distance;
}
