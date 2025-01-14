#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "asiGlider.h"

asiGlider::asiGlider(int xPos, int yPos, int size, const char *parentName) : instrument(xPos, yPos, size)
{
    if (parentName) {
        // Use position, size and vars from parent
        setName(parentName);
    }
    else {
        setName("ASI Glider");
        addVars();
    }

    simVars = &globals.simVars->simVars;
    resize();
}

/// <summary>
/// Destroy and recreate all bitmaps as instrument has been resized
/// </summary>
void asiGlider::resize()
{
    destroyBitmaps();

    // Create bitmaps scaled to correct size (original size is 800)
    scaleFactor = size / 800.0f;

    // 0 = Original (loaded) bitmap
    ALLEGRO_BITMAP* orig = loadBitmap("asi-glider.png");
    addBitmap(orig);

    if (bitmaps[0] == NULL) {
        return;
    }

    // 1 = Destination bitmap (all other bitmaps get assembled to here)
    ALLEGRO_BITMAP* bmp = al_create_bitmap(size, size);
    addBitmap(bmp);

    // 2 = Main dial
    bmp = al_create_bitmap(size, size);
    al_set_target_bitmap(bmp);
    al_draw_scaled_bitmap(orig, 0, 0, 800, 800, 0, 0, size, size, 0);
    addBitmap(bmp);

    // 3 = Pointer
    bmp = al_create_bitmap(46, 554);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 800, 28, 48, 554, 0, 0, 0);
    addBitmap(bmp);

    // 4 = Pointer shadow
    bmp = al_create_bitmap(46, 554);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 848, 28, 48, 554, 0, 0, 0);
    addBitmap(bmp);

    al_set_target_backbuffer(globals.display);
}

/// <summary>
/// Draw the instrument at the stored position
/// </summary>
void asiGlider::render()
{
    if (bitmaps[0] == NULL) {
        return;
    }

    // Use normal blender
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    // Draw stuff into dest bitmap
    al_set_target_bitmap(bitmaps[1]);

    // Add main dial
    al_draw_bitmap(bitmaps[2], 0, 0, 0);

    if (globals.enableShadows) {
        // Set blender to multiply (shades of grey darken, white has no effect)
        al_set_blender(ALLEGRO_ADD, ALLEGRO_DEST_COLOR, ALLEGRO_ZERO);

        // Add pointer shadow
        al_draw_scaled_rotated_bitmap(bitmaps[4], 24, 374, 410 * scaleFactor, 410 * scaleFactor, scaleFactor, scaleFactor, angle * DegreesToRadians, 0);

        // Restore normal blender
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
    }

    // Add pointer
    al_draw_scaled_rotated_bitmap(bitmaps[3], 24, 374, 400 * scaleFactor, 400 * scaleFactor, scaleFactor, scaleFactor, angle * DegreesToRadians, 0);

    // Position dest bitmap on screen
    al_set_target_backbuffer(globals.display);
    al_draw_bitmap(bitmaps[1], xPos, yPos, 0);

    if (!globals.electrics) {
        dimInstrument();
    }
}

/// <summary>
/// Fetch flightsim vars and then update all internal variables
/// that affect this instrument.
/// </summary>
void asiGlider::update()
{
    // Check for position or size change
    long *settings = globals.simVars->readSettings(name, xPos, yPos, size);

    xPos = settings[0];
    yPos = settings[1];

    if (size != settings[2]) {
        size = settings[2];
        resize();
    }

    // Calculate values
    kmph = simVars->asiAirspeed * 1.852;

    if (kmph < 20) {
        kmph = 20;
    }

    if (kmph <= 100) {
        angle = 124 + kmph * 2.3;
    }
    else if (kmph <= 150) {
        angle = 154 + kmph * 2.0;   // -206
    }
    else {
        angle = 217 + kmph * 1.58;  // -143
    }
}

/// <summary>
/// Add FlightSim variables for this instrument (used for simulation mode)
/// </summary>
void asiGlider::addVars()
{
    globals.simVars->addVar(name, "Airspeed Indicated", false, 1, 0);
}
