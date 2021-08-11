//
// Created by 16182 on 8/2/2021.
//

#ifndef HEART_SIM_VIEWSTATE_H
#define HEART_SIM_VIEWSTATE_H

#define GLM_FORCE_RADIANS
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
class ViewState {
public:
    ViewState():
            start_x_pos(),
            start_y_pos(),
            cursor_x_pos(),
            cursor_y_pos(),
            position({0, 0, -100, 0})
    {}
    ViewState(double cursor_x_pos, double cursor_y_pos):
            start_x_pos(cursor_x_pos),
            start_y_pos(cursor_y_pos),
            cursor_x_pos(cursor_x_pos),
            cursor_y_pos(cursor_y_pos),
            position({0, 0, -100, 0})
    {}
    [[nodiscard]] glm::mat4 mvp(int viewport_width, int viewport_height) const {
        auto model = glm::translate(rotation(), glm::vec3(position));
        auto projection = glm::perspective(glm::radians(45.f), float(viewport_width) / float(viewport_height), 0.1f, 1000.f);
        return projection * model;
    }

    void update_cursor_pos(double xpos, double ypos){
        cursor_x_pos = xpos;
        cursor_y_pos = ypos;
        if(cursor_y_pos - start_y_pos > 900) start_y_pos = cursor_y_pos - 900;
        if(cursor_y_pos - start_y_pos < -900) start_y_pos = cursor_y_pos + 900;
    }

    void update_key_press(bool forward, bool backward, bool left, bool right, bool up, bool down, float dt) {
        auto change = glm::vec4((int)left - (int)right, (int)up - (int)down, (int)forward - (int)backward, 0) * dt * 10.f;
        position += change * x_rotation();
    }

private:
    [[nodiscard]] glm::mat4 x_rotation() const {
        float dx = cursor_x_pos - start_x_pos;
        return glm::rotate(glm::mat4{1}, dx * glm::radians(0.1f), glm::vec3{0,1,0});
    }
    [[nodiscard]] glm::mat4 y_rotation() const {
        float dy = cursor_y_pos - start_y_pos;
        return glm::rotate(glm::mat4{1}, -dy * glm::radians(0.1f), glm::vec3{1, 0, 0});
    }
    [[nodiscard]] glm::mat4 rotation() const {
        return y_rotation() * x_rotation();
    }
    double start_x_pos, start_y_pos;
    double cursor_x_pos, cursor_y_pos;
    glm::vec4 position;
};

#endif //HEART_SIM_VIEWSTATE_H
