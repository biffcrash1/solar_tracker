#ifndef GRAPH_H
#define GRAPH_H

#include <Adafruit_SSD1306.h>
#include "param_config.h"

typedef struct 
{
  Adafruit_SSD1306* display;
  int buffer[SCREEN_WIDTH];
  int length;
} Graph_t;

// Function declarations
void Graph_init( Graph_t* graph, Adafruit_SSD1306* disp );
void Graph_addPoint( Graph_t* graph, int value );
void Graph_drawGraph( Graph_t* graph );

// Helper function
void Graph_draw( Graph_t* graph );

#endif // GRAPH_H
