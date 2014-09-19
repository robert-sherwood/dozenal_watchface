#include <pebble.h>

Window *window;
TextLayer *time_layer;
TextLayer *date_layer;
TextLayer *year_layer;
TextLayer *info_layer;
Layer *line_layer;

static const char DOZ_DIGITS[] = {
	'0','1','2','3','4','5','6','7','8','9','X','E'
};

static const char* const DOZ_PAIRS[] = {
	"00","01","02","03","04","05","06","07","08","09","0X","0E",
	"10","11","12","13","14","15","16","17","18","19","1X","1E",
	"20","21","22","23","24","25","26","27","28","29","2X","2E",
	"30","31","32","33","34","35","36","37","38","39","3X","3E",
	"40","41","42","43","44","45","46","47","48","49","4X","4E",
	"50","51","52","53","55","55","56","57","58","59","5X","5E",
	"60"
};

char * dec_to_doz_year(int dec_year) {
	static char date[5];
	int scratch[5];
	int i1, i2; 
	
	i1 = i2 = 0;
	
	// Calculate the dozenal values of the integer year at each position (works backwards)
	while (dec_year != 0) {
		scratch[i1] = dec_year % 12;
		dec_year /= 12;
		++i1;
	}
	
	// back up to last position
	--i1;
	
	// assign char to each position in year string 
	for ( ; i1 >= 0; i1--) {
		date[i2] = DOZ_DIGITS[scratch[i1]];
		i2++;
	}
	
	// Properly terminate year string
	date[i2] = '\0';
	
	return date;
}

void refresh_screen(struct tm *tick_time) {
	static char time_text[6];
	static char date_text[6];
	static char year_text[5];
	static int doz_mins;
	
	// calculate dates and times	
	doz_mins = ((tick_time->tm_min)*60+tick_time->tm_sec)/50;
	snprintf(time_text, 6, "%s:%s", DOZ_PAIRS[tick_time->tm_hour], DOZ_PAIRS[doz_mins]);
	snprintf(date_text, 11, "%s/%s", DOZ_PAIRS[tick_time->tm_mday], DOZ_PAIRS[tick_time->tm_mon+1]);
	snprintf(year_text, 5, "%s", dec_to_doz_year(tick_time->tm_year+1900));
	
	// update layers
	text_layer_set_text(time_layer, time_text);
	text_layer_set_text(date_layer, date_text);
	text_layer_set_text(year_layer, year_text);	
	
}

void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	if (((tick_time->tm_min)*60+tick_time->tm_sec)%50 == 0) {
		refresh_screen(tick_time);
	}
}

void handle_init(void) {
	// Create a window and text layers
	window = window_create();
		
	// Create the Time Layer, set the font, and text alignment
	time_layer = text_layer_create(GRect(0, 5, 144, 168));
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	
	// Add the time layer to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));

	//Create a horizontal divider
	line_layer = layer_create(GRect(8, 60, 128, 2));
	layer_set_update_proc(line_layer, line_layer_update_callback);
	layer_add_child(window_get_root_layer(window), line_layer);
	
	// Create the Date Layer, set the font, and text alignment
	date_layer = text_layer_create(GRect(0, 70, 144, 108));
	text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	
	// Add the date layer to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
	
	// Create the Year Layer, set the font, and text alignment
	year_layer = text_layer_create(GRect(0, 100, 144, 108));
	text_layer_set_font(year_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
	text_layer_set_text_alignment(year_layer, GTextAlignmentCenter);
	
	// Add the year layer to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(year_layer));	
	
	// Create the info layer, set the font, text and alignment
	info_layer = text_layer_create(GRect(0, 140, 144, 30));
	text_layer_set_font(info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(info_layer, GTextAlignmentCenter);
	text_layer_set_text(info_layer, "www.dozenal.org");
	
	// Add the info layer to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(info_layer));
	
	// Push the window
	window_stack_push(window, true);
	
	// initiate the screen
	time_t now = time(NULL);
	refresh_screen(localtime(&now));
	
	// Register the time handler function
	tick_timer_service_subscribe(SECOND_UNIT, handle_minute_tick);
	
	// App Logging!
	// APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}

void handle_deinit(void) {
	// Destroy the text layers
	text_layer_destroy(time_layer);
	text_layer_destroy(date_layer);
	text_layer_destroy(info_layer);
	text_layer_destroy(year_layer);
	layer_destroy(line_layer);
	
	// Unsubsribe from tick timer
	tick_timer_service_unsubscribe();
	
	// Destroy the window
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
