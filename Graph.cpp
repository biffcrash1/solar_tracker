#include "Graph.h"

//***********************************************************
//     Function Name: Graph_init
//
//     Inputs:
//     - graph : Pointer to Graph_t struct to initialize
//     - disp : Pointer to the SSD1306 display object
//
//     Returns:
//     - None
//
//     Description:
//     - Initializes the graph with a display pointer and resets
//       the data buffer length to 0.
//
//***********************************************************
void Graph_init( Graph_t* graph, Adafruit_SSD1306* disp )
{
  graph->display = disp;
  graph->length = 0;
}

//***********************************************************
//     Function Name: Graph_addPoint
//
//     Inputs:
//     - graph : Pointer to Graph_t struct
//     - value : Power value to add to the graph buffer
//
//     Returns:
//     - None
//
//     Description:
//     - Adds a new data point to the graph buffer. If the buffer
//       is full, shifts all values left and adds the new value
//       at the end.
//
//***********************************************************
void Graph_addPoint( Graph_t* graph, int value )
{
  const int width = SCREEN_WIDTH;
  if( graph->length < width )
  {
    graph->buffer[graph->length++] = value;
  }
  else
  {
    for( int i = 0; i < width - 1; i++ )
    {
      graph->buffer[i] = graph->buffer[i + 1];
    }
    graph->buffer[width - 1] = value;
  }
}

//***********************************************************
//     Function Name: Graph_drawGraph
//
//     Inputs:
//     - graph : Pointer to Graph_t struct
//
//     Returns:
//     - None
//
//     Description:
//     - Public interface to draw the graph. Calls the private
//       draw() method to render the graph on the display.
//
//***********************************************************
void Graph_drawGraph( Graph_t* graph )
{
  Graph_draw(graph);
}

//***********************************************************
//     Function Name: Graph_draw
//
//     Inputs:
//     - graph : Pointer to Graph_t struct
//
//     Returns:
//     - None
//
//     Description:
//     - Draws the power graph on the lower half of the display.
//       Scales the data to fit the display height and draws
//       connected line segments between data points.
//
//***********************************************************
void Graph_draw( Graph_t* graph )
{
  const int width = SCREEN_WIDTH;
  const int height = SCREEN_HEIGHT / 2;
  const int yOffset = SCREEN_HEIGHT / 2;

  graph->display->fillRect( 0, yOffset, width, height, SSD1306_BLACK );
  if( graph->length < 2 ) return;

  int maxVal = graph->buffer[0];
  for( int i = 1; i < graph->length; i++ )
  {
    if( graph->buffer[i] > maxVal ) maxVal = graph->buffer[i];
  }
  if( maxVal <= 0 ) return;

  float scale = ( height * GRAPH_SCALE_MARGIN ) / maxVal;
  for( int i = 1; i < graph->length; i++ )
  {
    int prevScaled = (int)( graph->buffer[i-1] * scale );
    int currScaled = (int)( graph->buffer[i] * scale );
    if( prevScaled > height ) prevScaled = height;
    if( currScaled > height ) currScaled = height;
    int x0 = i - 1;
    int y0 = yOffset + height - prevScaled;
    int x1 = i;
    int y1 = yOffset + height - currScaled;
    graph->display->drawLine( x0, y0, x1, y1, SSD1306_WHITE );
  }
}
