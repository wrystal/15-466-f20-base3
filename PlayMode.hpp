#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "Mesh.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <optional>


enum class CarModel {
    Police,
    Ambulance,
    TruckFlat,
    Sedan,
    Van,
};

const std::map<CarModel, std::string> CAR_MODEL_NAMES = {
        {CarModel::Police, "Police"},
        {CarModel::Ambulance, "Ambulance"},
        {CarModel::TruckFlat, "TruckFlat"},
        {CarModel::Sedan, "Van"},
        {CarModel::Van, "Van"},
};

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	static constexpr float LANE_WIDTH = 2.0;

	struct RoadTiles {
		RoadTiles(PlayMode *p, int num_tiles);
		static constexpr int ROAD_TILE_DEPTH = 2;
		void attachToDrawable();
		void detachFromDrawable();
		void update(float elapsed);
		PlayMode *p;
		int num_tiles;
		std::list<Scene::Transform> transforms;
		std::vector<decltype(Scene::drawables)::iterator> drawable_iterators;
		const Mesh *mesh;
	} road_tiles{this, 100};

	struct Player {
		Player(Scene *s);
		void goLeft() { target_lane_ = std::max(-1, target_lane_ - 1); }
		void goRight() { target_lane_ = std::min(1, target_lane_ + 1); };
		void attachToDrawable();
		void update(float elapsed);
		void enter_crash_phase();
		float position_ = 0;
		int target_lane_ = 0;
		bool crashed = false;
		const Mesh *mesh_ = nullptr;
		Scene *scene_;
		Scene::Transform transform_;
		static constexpr float PLAYER_SPEED = 5.0f;

		// crash related
		float sec_since_crash = 0.0f;
		static constexpr float INIT_Z_SPEED = 10.0f;
	    static constexpr float ROTATE_SPEED = 720.0f; // 720 degree/seconds
		void crash_animation(float elapsed);

	} player{&this->scene};

	struct OncomingCars {
	public:
		OncomingCars(Scene *s, Player *p);
		/**
		 * @return true if a collision happened, and the game should end
		 */
		bool update(float elapsed);
		/**
		 * Called when crash happens
		 */
		void mute_all_cars();
	private:
        struct Car {
            CarModel model;
			Scene::Transform t;
			int lane_; // -1, 0, 1
			std::optional<decltype(Scene::drawables)::iterator> it;
	        std::shared_ptr<Sound::PlayingSample> engine_sample;
	        std::shared_ptr<Sound::PlayingSample> horn_sample;
		};

		float next_car_interval_ = 1.0;
		float sound_position_multiplier = 10.0f;
		float car_disappear_y = -10.0f;
		void generate_new_car();
		void detach_obsolete_car(Car &c);
		std::list<Car> cars_;
		Scene *scene_ = nullptr;
		Player *player_ = nullptr;
	} oncoming_cars{&this->scene, &this->player};

	//----- game state -----


	static constexpr float LOW_BRIGHTNESS = 0.1f;
	static constexpr float MED_BRIGHTNESS = 0.4f;
	static constexpr float HIGH_BRIGHTNESS = 1.0f;
	float brightness = 1.0f;
	std::deque<std::pair<float, float>> brightness_animation = {
//		{HIGH_BRIGHTNESS, 10},
		{LOW_BRIGHTNESS, 8.0f}
	};

	void update_brightness(float elapsed);
	// record the bgm sample
	std::shared_ptr<Sound::PlayingSample> bgm_sample;

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//camera:
	Scene::Camera *camera = nullptr;

	float total_score = 0.0f; // total_elased
};
