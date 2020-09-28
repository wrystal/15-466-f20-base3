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
};

const std::map<CarModel, std::string> CAR_MODEL_NAMES = {
        {CarModel::Police, "Police"},
//        {CarModel::Ambulance, "Ambulance"},
        {CarModel::Ambulance, "Van"},
        {CarModel::TruckFlat, "TruckFlat"},
//        {CarModel::Sedan, "Sedan"},
        {CarModel::Sedan, "Van"},
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
		void detachFromDrawable(); // TODO(xiaoqiao): not implemented -- is this really needed?
		void update(float elapsed);
		float position_ = 0;
		int target_lane_ = 0;
		const Mesh *mesh_ = nullptr;
		Scene *scene_;
		Scene::Transform transform_;
		static constexpr int PLAYER_SPEED = 15;
	} player{&this->scene};

	struct OncomingCars {
	public:
		OncomingCars(Scene *s, Player *p);
		/**
		 * @return true if a collision happened, and the game should end
		 */
		bool update(float elapsed);
	private:
        struct Car {
            CarModel model;
			Scene::Transform t;
			int lane_;
			std::optional<decltype(Scene::drawables)::iterator> it;
            std::shared_ptr<Sound::PlayingSample> playingSample;
		};
		float next_car_interval_ = 1.0;
		void generate_new_car();
		void detach_obsolete_car(Car &c);
		std::list<Car> cars_;
		Scene *scene_ = nullptr;
		Player *player_ = nullptr;
	} oncoming_cars{&this->scene, &this->player};

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
//	Scene::Transform *hip = nullptr;
//	Scene::Transform *upper_leg = nullptr;
//	Scene::Transform *lower_leg = nullptr;
//	glm::quat hip_base_rotation;
//	glm::quat upper_leg_base_rotation;
//	glm::quat lower_leg_base_rotation;
//	float wobble = 0.0f;

//	glm::vec3 get_leg_tip_position();

	//music coming from the tip of the leg (as a demonstration):
//	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
