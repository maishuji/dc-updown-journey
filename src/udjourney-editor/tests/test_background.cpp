// Copyright 2025 Quentin Cartier
#include <gtest/gtest.h>

#include <string>

#include "udjourney-editor/background/BackgroundLayer.hpp"
#include "udjourney-editor/background/BackgroundManager.hpp"

// Test BackgroundLayer basic functionality
TEST(BackgroundLayerTest, ConstructorInitializesFields) {
    BackgroundLayer layer("Sky", 0.5f, 0);

    EXPECT_EQ(layer.get_name(), "Sky");
    EXPECT_FLOAT_EQ(layer.get_parallax_factor(), 0.5f);
    EXPECT_EQ(layer.get_depth(), 0);
    EXPECT_EQ(layer.get_objects().size(), 0);
}

TEST(BackgroundLayerTest, AddAndRemoveObjects) {
    BackgroundLayer layer("Mountains", 0.7f, 1);

    BackgroundObject obj1{"tree", 100.0f, 200.0f, 1.0f, 0.0f};
    BackgroundObject obj2{"rock", 300.0f, 250.0f, 0.8f, 15.0f};

    layer.add_object(obj1);
    layer.add_object(obj2);

    EXPECT_EQ(layer.get_objects().size(), 2);
    EXPECT_EQ(layer.get_objects()[0].sprite_name, "tree");
    EXPECT_FLOAT_EQ(layer.get_objects()[0].x, 100.0f);
    EXPECT_FLOAT_EQ(layer.get_objects()[1].scale, 0.8f);

    layer.remove_object(0);
    EXPECT_EQ(layer.get_objects().size(), 1);
    EXPECT_EQ(layer.get_objects()[0].sprite_name, "rock");

    layer.clear_objects();
    EXPECT_EQ(layer.get_objects().size(), 0);
}

TEST(BackgroundLayerTest, Setters) {
    BackgroundLayer layer("Test", 0.5f, 0);

    layer.set_name("NewName");
    EXPECT_EQ(layer.get_name(), "NewName");

    layer.set_texture_file("new.png");
    EXPECT_EQ(layer.get_texture_file(), "new.png");

    layer.set_parallax_factor(0.9f);
    EXPECT_FLOAT_EQ(layer.get_parallax_factor(), 0.9f);

    layer.set_depth(5);
    EXPECT_EQ(layer.get_depth(), 5);
}

// Test BackgroundManager functionality
TEST(BackgroundManagerTest, AddLayer) {
    BackgroundManager manager;

    BackgroundLayer layer("Sky", 0.3f, 0);
    layer.set_texture_file("sky.png");

    EXPECT_TRUE(manager.add_layer(layer));
    EXPECT_EQ(manager.get_layers().size(), 1);
    EXPECT_EQ(manager.get_layers()[0].get_name(), "Sky");
}

TEST(BackgroundManagerTest, MaxLayersConstraint) {
    BackgroundManager manager;

    // Add maximum allowed layers
    for (int i = 0; i < 5; ++i) {
        BackgroundLayer layer("Layer" + std::to_string(i), 0.5f, i);
        EXPECT_TRUE(manager.add_layer(layer));
    }

    // Attempt to add one more should fail
    BackgroundLayer extra("ExtraLayer", 0.5f, 5);
    EXPECT_FALSE(manager.add_layer(extra));
    EXPECT_EQ(manager.get_layers().size(), 5);
}

TEST(BackgroundManagerTest, RemoveLayer) {
    BackgroundManager manager;

    BackgroundLayer layer1("Layer1", 0.3f, 0);
    BackgroundLayer layer2("Layer2", 0.5f, 1);
    BackgroundLayer layer3("Layer3", 0.7f, 2);

    manager.add_layer(layer1);
    manager.add_layer(layer2);
    manager.add_layer(layer3);

    manager.remove_layer(1);
    EXPECT_EQ(manager.get_layers().size(), 2);
    EXPECT_EQ(manager.get_layers()[0].get_name(), "Layer1");
    EXPECT_EQ(manager.get_layers()[1].get_name(), "Layer3");
}

TEST(BackgroundManagerTest, MoveLayer) {
    BackgroundManager manager;

    BackgroundLayer layer0("Layer0", 0.3f, 0);
    BackgroundLayer layer1("Layer1", 0.5f, 1);
    BackgroundLayer layer2("Layer2", 0.7f, 2);

    manager.add_layer(layer0);
    manager.add_layer(layer1);
    manager.add_layer(layer2);

    manager.move_layer_down(0);  // Move Layer0 down
    EXPECT_EQ(manager.get_layers()[0].get_name(), "Layer1");
    EXPECT_EQ(manager.get_layers()[1].get_name(), "Layer0");
    EXPECT_EQ(manager.get_layers()[2].get_name(), "Layer2");

    manager.move_layer_up(1);  // Move Layer0 back up
    EXPECT_EQ(manager.get_layers()[0].get_name(), "Layer0");
    EXPECT_EQ(manager.get_layers()[1].get_name(), "Layer1");
    EXPECT_EQ(manager.get_layers()[2].get_name(), "Layer2");
}

TEST(BackgroundManagerTest, LayerSelection) {
    BackgroundManager manager;

    BackgroundLayer layer1("Layer1", 0.3f, 0);
    BackgroundLayer layer2("Layer2", 0.5f, 1);

    manager.add_layer(layer1);
    manager.add_layer(layer2);

    EXPECT_FALSE(manager.get_selected_layer().has_value());

    manager.select_layer(0);
    EXPECT_TRUE(manager.get_selected_layer().has_value());
    EXPECT_EQ(manager.get_selected_layer().value(), 0);

    manager.clear_selection();
    EXPECT_FALSE(manager.get_selected_layer().has_value());
}

TEST(BackgroundManagerTest, AddObjectToLayer) {
    BackgroundManager manager;

    BackgroundLayer layer("Layer1", 0.3f, 0);
    manager.add_layer(layer);

    BackgroundObject obj{"tree", 100.0f, 200.0f, 1.0f, 0.0f};
    manager.add_object(0, obj);
    EXPECT_EQ(manager.get_layers()[0].get_objects().size(), 1);
}

TEST(BackgroundManagerTest, RemoveObjectFromLayer) {
    BackgroundManager manager;

    BackgroundLayer layer("Layer1", 0.3f, 0);
    manager.add_layer(layer);

    BackgroundObject obj1{"tree", 100.0f, 200.0f, 1.0f, 0.0f};
    BackgroundObject obj2{"rock", 300.0f, 250.0f, 0.8f, 15.0f};

    manager.add_object(0, obj1);
    manager.add_object(0, obj2);

    manager.remove_object(0, 0);
    EXPECT_EQ(manager.get_layers()[0].get_objects().size(), 1);
}

TEST(BackgroundManagerTest, JSONSerialization) {
    BackgroundManager manager;

    // Create test data
    BackgroundLayer sky("Sky", 0.3f, 0);
    sky.set_texture_file("sky.png");
    BackgroundObject cloud{"cloud1", 100.0f, 50.0f, 1.0f, 0.0f};
    sky.add_object(cloud);

    BackgroundLayer mountains("Mountains", 0.6f, 1);
    mountains.set_texture_file("mountains.png");
    BackgroundObject tree{"tree", 200.0f, 300.0f, 1.2f, 10.0f};
    mountains.add_object(tree);

    manager.add_layer(sky);
    manager.add_layer(mountains);

    // Save to file
    std::string test_file = "test_background.json";
    manager.save_to_file(test_file);

    // Create new manager and load
    BackgroundManager loaded_manager;
    loaded_manager.load_from_file(test_file);

    // Verify loaded data
    EXPECT_EQ(loaded_manager.get_layers().size(), 2);
    EXPECT_EQ(loaded_manager.get_layers()[0].get_name(), "Sky");
    EXPECT_FLOAT_EQ(loaded_manager.get_layers()[0].get_parallax_factor(), 0.3f);
    EXPECT_EQ(loaded_manager.get_layers()[0].get_objects().size(), 1);
    EXPECT_EQ(loaded_manager.get_layers()[0].get_objects()[0].sprite_name,
              "cloud1");

    EXPECT_EQ(loaded_manager.get_layers()[1].get_name(), "Mountains");
    EXPECT_EQ(loaded_manager.get_layers()[1].get_objects().size(), 1);
    EXPECT_FLOAT_EQ(loaded_manager.get_layers()[1].get_objects()[0].scale,
                    1.2f);

    // Clean up
    std::remove(test_file.c_str());
}

TEST(BackgroundManagerTest, DepthSorting) {
    BackgroundManager manager;

    // Add layers in random depth order
    BackgroundLayer far("Far", 0.2f, 5);
    BackgroundLayer near("Near", 0.9f, 1);
    BackgroundLayer middle("Middle", 0.5f, 3);

    manager.add_layer(far);
    manager.add_layer(near);
    manager.add_layer(middle);

    // Layers should be sorted by depth (lower depth renders first/behind)
    EXPECT_EQ(manager.get_layers()[0].get_depth(), 1);
    EXPECT_EQ(manager.get_layers()[1].get_depth(), 3);
    EXPECT_EQ(manager.get_layers()[2].get_depth(), 5);
}
