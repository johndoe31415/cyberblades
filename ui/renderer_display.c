static void swbuf_render_small_display(const struct server_state_t *server_state, struct cairo_swbuf_t *swbuf) {
	swbuf_clear(swbuf, COLOR_BS_DARKBLUE);
	if (server_state->ui_screen == MAIN_SCREEN) {
		const int cyberblades_offset = -5;
		swbuf_text(swbuf, &(const struct font_placement_t) {
			.font_face = "Beon",
			.font_size = 32,
			.font_color = COLOR_BS_RED,
			.placement = {
				.src_anchor = {
					.x = XPOS_RIGHT,
					.y = YPOS_BOTTOM,
				},
				.dst_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_TOP,
				},
				.yoffset = 32,
				.xoffset = -5 + cyberblades_offset,
			}
		}, "Cyber");
		swbuf_text(swbuf, &(const struct font_placement_t) {
			.font_face = "Beon",
			.font_size = 32,
			.font_color = COLOR_BS_BLUE,
			.placement = {
				.src_anchor = {
					.x = XPOS_LEFT,
					.y = YPOS_BOTTOM,
				},
				.dst_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_TOP,
				},
				.yoffset = 32,
				.xoffset = 5 + cyberblades_offset,
			}
		}, "Blades");

		{
			struct font_placement_t text_placement = {
				.font_face = "Roboto",
				.font_size = 16,
				.font_color = COLOR_WHITE,
				.placement = {
					.src_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_BOTTOM,
					},
					.dst_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_BOTTOM,
					},
					.yoffset = -10,
				}
			};
			const struct anchored_placement_t rect_placement = {
				.src_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_BOTTOM,
				},
				.dst_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_BOTTOM,
				},
				.yoffset = -3,
			};
			switch (server_state->historian->connection_state) {
				case UNCONNECTED:
					swbuf_rect(swbuf, &(const struct rect_placement_t){
						.placement = rect_placement,
						.color = COLOR_POMEGRANATE,
						.fill = true,
						.round = 10,
						.width = 200,
						.height = 25,
					});
					swbuf_text(swbuf, &text_placement, "Historian unavailable");
					break;

				case CONNECTED_WAITING:
					swbuf_rect(swbuf, &(const struct rect_placement_t){
						.placement = rect_placement,
						.color = COLOR_SUN_FLOWER,
						.fill = true,
						.round = 10,
						.width = 200,
						.height = 25,
					});
					text_placement.font_color = COLOR_BLACK;
					swbuf_text(swbuf, &text_placement, "Unconnected");
					break;

				case CONNECTED_READY:
					swbuf_rect(swbuf, &(const struct rect_placement_t){
						.placement = rect_placement,
						.color = COLOR_EMERLAND,
						.fill = true,
						.round = 10,
						.width = 200,
						.height = 25,
					});
					swbuf_text(swbuf, &text_placement, "Ready for action");
					break;
			}
		}

		if (server_state->current_player[0]) {
			swbuf_text(swbuf, &(const struct font_placement_t){
				.font_face = "Roboto",
				.font_size = 22,
				.font_color = COLOR_SILVER,
				.placement = {
					.src_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_BOTTOM,
					},
					.dst_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_TOP,
					},
					.yoffset = 65 + 25 * 0,
				}

			}, "Player: %s", server_state->current_player);
		} else {
			swbuf_text(swbuf, &(const struct font_placement_t){
				.font_face = "Roboto",
				.font_size = 22,
				.font_color = COLOR_POMEGRANATE,
				.placement = {
					.src_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_BOTTOM,
					},
					.dst_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_TOP,
					},
					.yoffset = 65 + 25 * 0,
				}

			}, "No player selected");
		}
		swbuf_text(swbuf, &(const struct font_placement_t){
			.font_face = "Roboto",
			.font_size = 22,
			.font_color =  COLOR_SILVER,
			.placement = {
				.src_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_BOTTOM,
				},
				.dst_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_TOP,
				},
				.yoffset = 65 + 25 * 1,
			}

		}, "Playtime: %2d:%02d", server_state->current_playtime / 60, server_state->current_playtime % 60);

		swbuf_text(swbuf, &(const struct font_placement_t){
			.font_face = "Roboto",
			.font_size = 22,
			.font_color =  COLOR_SILVER,
			.placement = {
				.src_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_BOTTOM,
				},
				.dst_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_TOP,
				},
				.yoffset = 65 + 25 * 2,
			}

		}, "Scoresum: %.1f k", server_state->score_sum / 1000.);

	} if (server_state->ui_screen == GAME_SCREEN) {
		{
			const struct font_placement_t placement = {
				.font_face = "Beon",
				.font_size = 32,
				.font_color = COLOR_BS_RED,
				.placement = {
					.src_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_BOTTOM,
					},
					.dst_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_TOP,
					},
					.yoffset = 32,
				}
			};
			swbuf_text(swbuf, &placement, "Game On");
		}
		swbuf_text(swbuf, &(const struct font_placement_t){
			.font_face = "Digital Dream Fat",
			.font_size = 24,
			.font_color = COLOR_SUN_FLOWER,
			.placement = {
				.src_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_BOTTOM,
				},
				.dst_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_TOP,
				},
				.yoffset = 65,
			}
		}, "%ld", server_state->current_score);
		swbuf_text(swbuf, &(const struct font_placement_t){
			.font_face = "Digital Dream Fat",
			.font_size = 20,
			.font_color = COLOR_ORANGE,
			.placement = {
				.src_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_BOTTOM,
				},
				.dst_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_TOP,
				},
				.yoffset = 65 + 32,
			}
		}, "%.1f%%", server_state->current_maxscore ? 100. * server_state->current_score / server_state->current_maxscore : 0);
	} if (server_state->ui_screen == FINISH_SCREEN) {
	}
}
