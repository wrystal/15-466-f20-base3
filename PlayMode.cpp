#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include "random.hpp"

using Random = effolkronium::random_static;

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("cars.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("cars.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		if (mesh_name=="TruckFlat" || mesh_name=="Van" || mesh_name=="Police" || mesh_name == "RoadTile") {
			return;
		}

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

//Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
//	return new Sound::Sample(data_path("dusty-floor.opus"));
//});

Load< std::map<CarModel, Sound::Sample>> horn_samples(LoadTagDefault, []() -> std::map<CarModel, Sound::Sample> const * {
	auto map_p = new std::map<CarModel, Sound::Sample>();

	// load horn sound
	map_p->emplace(CarModel::Sedan, Sound::Sample(data_path("SedanHorn.opus")));
	map_p->emplace(CarModel::Police, Sound::Sample(data_path("PoliceHorn.opus")));
	map_p->emplace(CarModel::Ambulance, Sound::Sample(data_path("AmbulanceHorn.opus")));
	map_p->emplace(CarModel::TruckFlat, Sound::Sample(data_path("TruckFlatHorn.opus")));

	return map_p;
});

Load< std::map<CarModel, Sound::Sample>> engine_samples(LoadTagDefault, []() -> std::map<CarModel, Sound::Sample> const * {
    auto map_p = new std::map<CarModel, Sound::Sample>();

    map_p->emplace(CarModel::Sedan, Sound::Sample(data_path("car_engine_2.opus")));
    map_p->emplace(CarModel::Police, Sound::Sample(data_path("car_engine_2.opus")));
    map_p->emplace(CarModel::Ambulance, Sound::Sample(data_path("car_engine_2.opus")));
    map_p->emplace(CarModel::TruckFlat, Sound::Sample(data_path("TruckFlatEngine.opus")));

    return map_p;
});

Load< Sound::Sample > background_music_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("bgm.opus"));
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:
//	for (auto &transform : scene.transforms) {
//		if (transform.name == "Hip.FL") hip = &transform;
//		else if (transform.name == "UpperLeg.FL") upper_leg = &transform;
//		else if (transform.name == "LowerLeg.FL") lower_leg = &transform;
//	}
//	if (hip == nullptr) throw std::runtime_error("Hip not found.");
//	if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
//	if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");
//
//	hip_base_rotation = hip->rotation;
////	upper_leg_base_rotation = upper_leg->rotation;
//	lower_leg_base_rotation = lower_leg->rotation;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//start music loop playing:
	// (note: position will be over-ridden in update())
//	leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_leg_tip_position(), 10.0f);
	Sound::loop(*background_music_sample, 0.1f, 0.0f);
	road_tiles.attachToDrawable();
	player.attachToDrawable();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
//	wobble += elapsed / 10.0f;
//	wobble -= std::floor(wobble);

//	hip->rotation = hip_base_rotation * glm::angleAxis(
//		glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
//		glm::vec3(0.0f, 1.0f, 0.0f)
//	);
//	upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
//		glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
//		glm::vec3(0.0f, 0.0f, 1.0f)
//	);
//	lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
//		glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
//		glm::vec3(0.0f, 0.0f, 1.0f)
//	);

	//move sound to follow leg tip position:
//	leg_tip_loop->set_position(get_leg_tip_position(), 1.0f / 60.0f);
	road_tiles.update(elapsed);
	player.update(elapsed);
	oncoming_cars.update(elapsed);
	updateBrightness(elapsed);
	{
		if (left.pressed && left.downs) {
			player.goLeft();
		}
		if (right.pressed && right.downs) {
			player.goRight();
		}
	}

	//move camera:
	{

//		//combine inputs into a move:
//		constexpr float PlayerSpeed = 30.0f;
//		glm::vec2 move = glm::vec2(0.0f);
//		if (left.pressed && !right.pressed) move.x =-1.0f;
//		if (!left.pressed && right.pressed) move.x = 1.0f;
//		if (down.pressed && !up.pressed) move.y =-1.0f;
//		if (!down.pressed && up.pressed) move.y = 1.0f;
//
//		//make it so that moving diagonally doesn't go faster:
//		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;
//
//		glm::mat4x3 frame = camera->transform->make_local_to_parent();
//		glm::vec3 right = frame[0];
//		//glm::vec3 up = frame[1];
//		glm::vec3 forward = -frame[2];
//
//		camera->transform->position += move.x * right + move.y * forward;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		glm::vec3 at = frame[3];
		Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f * brightness, 1.0f * brightness, 0.95f * brightness)));
	glUseProgram(0);

	glClearColor(0.5f * brightness, 0.5f * brightness, 0.5f * brightness, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}
void PlayMode::updateBrightness(float elapsed) {
	if (!brightness_animation.empty()) {
//		std::cout << "elapsed: " << elapsed << std::endl;
//		for (auto p : brightness_animation) {
//			std::cout << p.first << ' ' << p.second << '\n';
//		}
		const float &target_val = brightness_animation.front().first;
		float &transition_time = brightness_animation.front().second;
		transition_time -= elapsed;
		if (transition_time <= 0) {
			brightness_animation.pop_front();
			return;
		}
		float speed = (target_val - brightness)/transition_time;
		float new_brightness = brightness + speed*elapsed;
		// if the difference is small enough, or if
		// it has overshoot,
		if (std::abs(new_brightness - target_val) < 0.01 ||
			std::signbit(target_val - brightness)!=std::signbit(target_val - new_brightness)) {
			new_brightness = target_val;
		}
		brightness = new_brightness;
	} else {
		// Add Random lightnings
		brightness_animation.emplace_back(LOW_BRIGHTNESS, Random::get(15.0, 30.0));
		for (int i = 0; i < Random::get<int>(1, 2); i++) {
			brightness_animation.emplace_back(MED_BRIGHTNESS, 0.1);
			brightness_animation.emplace_back(MED_BRIGHTNESS, 0.1);
			brightness_animation.emplace_back(LOW_BRIGHTNESS, 0.1);
			brightness_animation.emplace_back(LOW_BRIGHTNESS, Random::get(0.05, 0.2));
		}
	}
}

//glm::vec3 PlayMode::get_leg_tip_position() {
//	//the vertex position here was read from the model in blender:
//	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
//}

PlayMode::RoadTiles::RoadTiles(PlayMode *p, int num_tiles) : p{p}, num_tiles{num_tiles} {
	mesh = &hexapod_meshes->lookup("RoadTile"); // TODO(xiaoqiao): the real name?
	for (int i=0; i<num_tiles; i++) {
		transforms.emplace_back();
		Scene::Transform &t = transforms.back();
		t.position = {0, i * ROAD_TILE_DEPTH - 10, 0};
		t.rotation = {1, 0, 0, 0};
	}
}

void PlayMode::RoadTiles::attachToDrawable() {
	int current_idx = 0;
	for (auto &t : transforms) {
		p->scene.drawables.emplace_back(&t);
		auto &d = p->scene.drawables.back();
		d.pipeline = lit_color_texture_program_pipeline;
		d.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		d.pipeline.start = mesh->start;
		d.pipeline.count = mesh->count;
		this->drawable_iterators.push_back(std::prev(p->scene.drawables.end()));
		current_idx++;
	}
}


void PlayMode::RoadTiles::detachFromDrawable() {
	assert(!drawable_iterators.empty());
	for (const auto it : drawable_iterators) {
		p->scene.drawables.erase(it);
	}
	drawable_iterators.clear();
}

void PlayMode::RoadTiles::update(float elapsed) {
	for (Scene::Transform &t : transforms) {
		t.position.y -= 10*elapsed;
		if (t.position.y <= -50) {
			t.position.y += num_tiles*ROAD_TILE_DEPTH;
		}
	}
}

PlayMode::Player::Player(Scene *s) : scene_{s} {
	transform_.position = {0.0f, -5.0f, 0.1f};
	transform_.rotation = glm::angleAxis<float>(
		glm::radians((float)180.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	mesh_ = &hexapod_meshes->lookup("TruckFlat");

}

void PlayMode::Player::attachToDrawable() {
	scene_->drawables.emplace_back(&transform_);
	auto &d = scene_->drawables.back();
	d.pipeline = lit_color_texture_program_pipeline;
	d.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
	d.pipeline.start = mesh_->start;
	d.pipeline.count = mesh_->count;
}

void PlayMode::Player::update(float elapsed) {
	float target_position = (float) target_lane_ * LANE_WIDTH;
	if (std::abs(target_position - position_) <= PLAYER_SPEED*elapsed) {
		position_ = target_position;
	} else {
		if (target_position > position_) {
			position_ += PLAYER_SPEED*elapsed;
		} else {
			position_ -= PLAYER_SPEED*elapsed;
		}
	}
	transform_.position.x = position_;
}

PlayMode::OncomingCars::OncomingCars(Scene *s, PlayMode::Player *p) {
	this->scene_ = s;
	this->player_ = p;
}

bool PlayMode::OncomingCars::update(float elapsed) {
	next_car_interval_ -= elapsed;
	if (next_car_interval_ <= 0) {
		generate_new_car();
		next_car_interval_ = Random::get(3.0f, 6.0f);
	}
	constexpr float ONCOMING_CAR_SPEED = 20.0;
	for (auto &car : cars_) {
		car.t.position.y -= elapsed*ONCOMING_CAR_SPEED;
		// update engine sound
		glm::vec3 sound_position = car.t.position;
		sound_position.x = float(car.t.position.x - player_->position_) * sound_position_multiplier;
		car.engineSample->set_position(sound_position);
		car.hornSample->set_position(sound_position);

		if (fabs(car.t.position.x - player_->position_) <= 0.2 &&
			fabs(player_->transform_.position.y - car.t.position.y) <= 50.0f) {
			// if player is on the same lane of the oncoming car, and they are close, horn
			car.hornSample->set_volume(1.0f);
 		} else {
			car.hornSample->set_volume(0.0f);
		}
	}

	// remove unused cars
	for (auto it = cars_.begin(); it != cars_.end();) {
		if (it->t.position.y < car_disappear_y) {
			detach_obsolete_car(*it);
			it = cars_.erase(it);
			continue;
		} else {
			it++;
		}
	}

	// is there any car collision?
	for (auto &car : cars_) {
		if (car.lane_==player_->target_lane_
			&& std::abs(car.t.position.y - player_->transform_.position.y) < 2) {
			// collision detected
			return true;
		}
	}
	return false;
}

void PlayMode::OncomingCars::generate_new_car() {
	cars_.emplace_back();
	Car &c = cars_.back();
	c.lane_ = Random::get(-1, 1);
	c.t.position.x = (float) c.lane_*LANE_WIDTH;
	c.t.position.y = 150.0f;
	c.t.position.z = 0.1f;

	scene_->drawables.emplace_back(&c.t);
	auto back_iterator = std::prev(scene_->drawables.end());
	Scene::Drawable &d = *back_iterator;
	c.it = std::make_optional(back_iterator);
	d.pipeline = lit_color_texture_program_pipeline;
	d.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;

	c.model = Random::get({
			                      CarModel::Police,
			                      CarModel::Ambulance,
			                      CarModel::TruckFlat,
			                      CarModel::Sedan
	                      });

	const Mesh *mesh_ = &hexapod_meshes->lookup(CAR_MODEL_NAMES.at(c.model));
	d.pipeline.start = mesh_->start;
	d.pipeline.count = mesh_->count;

	//add horn sound to this car
//	const Sound::Sample &horn_sample = (*horn_samples).at(c.model);
//	glm::vec3 sound_position = c.t.position;
//	sound_position.x = float(c.t.position.x - player_->position_) * sound_position_multiplier;
	const Sound::Sample &engine_sample = (*engine_samples).at(c.model);
	const Sound::Sample &horn_sample = (*horn_samples).at(c.model);

	glm::vec3 sound_position = c.t.position;
	sound_position.x = float(c.t.position.x - player_->position_) * sound_position_multiplier;
	c.engineSample = Sound::loop_3D(engine_sample, 1.0f, sound_position, 4.0f);
	/* Set it to silence */
	c.hornSample = Sound::loop_3D(horn_sample, 0.0f, sound_position, 4.0f);
}

void PlayMode::OncomingCars::detach_obsolete_car(Car &c) {
	scene_->drawables.erase(c.it.value());
	c.it = std::nullopt;
	if(c.engineSample) {
		c.engineSample->stop(2);
	}
	if(c.hornSample) {
		c.hornSample->stop(2);
	}
	c.engineSample = nullptr;
	c.hornSample = nullptr;
}
