#include "pebble.h"
#include "pebble_fonts.h"

static Window *window;
static GFont font_small;
static GFont font_big;
static InverterLayer *inverter;
static PropertyAnimation inverter_anim;

typedef struct {
	TextLayer * layer;
	PropertyAnimation anim;
	const char * text;
	const char * old_text;
} word_t;

static word_t first_word;
static word_t first_word_between;
static word_t second_word;
static word_t second_word_between;
static word_t third_word;
static const char *hours[] = {"twaalf","een","twee","drie","vier","vijf","zes","zeven","acht","negen","tien","elf","twaalf"};

void text_layer_setup(Window * window, TextLayer * layer, GRect frame, GFont font) {
	layer = text_layer_create(frame);
	text_layer_set_text(layer, "");
	text_layer_set_text_color(layer, GColorWhite);
	text_layer_set_background_color(layer, GColorClear);
	text_layer_set_text_alignment(layer, GTextAlignmentCenter);
	text_layer_set_font(layer, font);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(layer));
}

static const char *hour_string(int h) {
	if(h > 11) {
		h -= 12;
	}
	return hours[h];
}
static const char *min_string(int m) {
	if(m == 5) {
		return "vijf";
	} else {
		return "tien";
	}
}

static void update_word(word_t * const word) {
	text_layer_set_text(word->layer, word->text);
	if(word->text != word->old_text) {
		animation_schedule(&word->anim.animation);
	}
}

static void nederlands_format(int h, int m) {
	first_word.text = "";
	first_word_between.text = "";
	second_word.text = "";
	second_word_between.text = "";
	third_word.text = "";
	
	int unrounded = m;
	float temp_m = m;
	temp_m = temp_m / 5;
	m = temp_m + 0.5;
	m = m * 5;
	
	if(m == 0 || m == 60) {
		if(m == 60) {
			h++;
		}
		first_word_between.text = hour_string(h);
		second_word_between.text = "uur";
	} else if(m == 30) {
		first_word_between.text = "half";
		second_word_between.text = hour_string(h + 1);
	} else if(m == 15) {
		first_word.text = "kwart";
		second_word.text = "over";
		third_word.text = hour_string(h);
	} else if(m == 45) {
		first_word.text = "kwart";
		second_word.text = "voor";
		third_word.text = hour_string(h + 1);
	} else if(m > 30) {
		if(m < 45) {
			first_word.text = min_string(m - 30);
			second_word.text = "over half";
			third_word.text = hour_string(h + 1);
		} else {
			first_word.text = min_string(60 - m);
			second_word.text = "voor";
			third_word.text = hour_string(h + 1);
		}
	} else {
		if(m > 15) {
			first_word.text = min_string(30 - m);
			second_word.text = "voor half";
			third_word.text = hour_string(h + 1);
		} else {
			first_word.text = min_string(m);
			second_word.text = "over";
			third_word.text = hour_string(h);
		}
	}
	
	GRect frame;
	GRect frame_right;
	switch(unrounded - m) {
		case -2:
		frame = GRect(116, 167, 28, 1);
		frame_right = frame;
		frame_right.origin.x = 0;
		break;
		case -1:
		frame = GRect(0, 167, 28, 1);
		frame_right = frame;
		frame_right.origin.x = 28;
		break;
		case 0:
		frame = GRect(28, 167, 28, 1);
		frame_right = frame;
		frame_right.origin.x = 56;
		frame_right.size.w = 32;
		break;
		case 1:
		frame = GRect(56, 167, 32, 1);
		frame_right = frame;
		frame_right.origin.x = 84;
		frame_right.size.w = 28;
		break;
		case 2:
		frame = GRect(84, 167, 28, 1);
		frame_right = frame;
		frame_right.origin.x = 116;
		break;
	}
	property_animation_create_layer_frame(inverter_layer_get_layer(inverter), &frame, &frame_right);
	animation_schedule(&inverter_anim.animation);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
	
	first_word.old_text = first_word.text;
	first_word_between.old_text = first_word_between.text;
	second_word.old_text = second_word.text;
	second_word_between.old_text = second_word_between.text;
	third_word.old_text = third_word.text;
	
	nederlands_format(tick_time->tm_hour,  tick_time->tm_min);
	
	update_word(&first_word);
	update_word(&first_word_between);
	update_word(&second_word);
	update_word(&second_word_between);
	update_word(&third_word);
}

static void text_layer(word_t * word, GRect frame, GFont font) {
	text_layer_setup(window, word->layer, frame, font);
	
	GRect frame_right = frame;
	frame_right.origin.x = 150;
	
	property_animation_create_layer_frame(text_layer_get_layer(word->layer), &frame_right, &frame);
	
	animation_set_duration(&word->anim.animation, 500);
	animation_set_curve(&word->anim.animation, AnimationCurveEaseIn);
	
	animation_set_duration(&inverter_anim.animation, 500);
	animation_set_curve(&inverter_anim.animation, AnimationCurveEaseIn);
}

static void init() {
	
	window = window_create();
	window_stack_push(window, false);
	window_set_background_color(window, GColorBlack);
	
	font_big = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_40));
	font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_30));
	
	text_layer(&first_word, GRect(0, 12, 143, 48), font_big);
	text_layer(&second_word, GRect(0, 62, 143, 42), font_small);
	text_layer(&third_word, GRect(0, 96, 143, 48), font_big);
	
	text_layer(&first_word_between, GRect(0, 27, 143, 48), font_big);
	text_layer(&second_word_between, GRect(0, 83, 143, 48), font_big);
	
	inverter = inverter_layer_create(GRect(0, 166, 36, 1));
	layer_add_child(window_get_root_layer(window), (Layer *)&inverter);
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
}

static void deinit() {
	fonts_unload_custom_font(font_small);
	fonts_unload_custom_font(font_big);
	window_destroy(window);
	tick_timer_service_unsubscribe();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}