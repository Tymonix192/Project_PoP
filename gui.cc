//contributors: 399554 397957
#include <iostream>
#include <cassert>
#include "graphic_gui.h"
#include "gui.h"

using namespace std;

enum Response
{
    CANCEL,
    OPEN,
    SAVE
};

// enum pour noms de boutons
enum ButtonName
{
    B_EXIT,
    B_OPEN,
    B_SAVE,
    B_RESTART,
    B_START,
    B_STEP
};

constexpr unsigned taille_dessin(500);

My_window::My_window(string file_name, Jeu* jeu)
    : main_box(Gtk::Orientation::HORIZONTAL),
      panel_box(Gtk::Orientation::VERTICAL),
      command_box(Gtk::Orientation::VERTICAL),
      command_frame("General"),
      info_frame("Info : nombre de..."),
      activated(false),
      buttons({Gtk::Button("exit"), Gtk::Button("open"), Gtk::Button("save"), 
        Gtk::Button("restart"), Gtk::Button("start"), Gtk::Button("step")}),
      checks({Gtk::CheckButton("Construction"), Gtk::CheckButton("Guidage")}),
      info_text({Gtk::Label("score:"),
                 Gtk::Label("particules:"),
                 Gtk::Label("faiseurs:"),
                 Gtk::Label("articulations:")}),
      previous_file_name(file_name),
     _jeu(jeu)
{
    set_title("Linked-Crossing Challenge");
    set_child(main_box);
    main_box.append(panel_box);
    main_box.append(drawing);
    panel_box.append(command_frame);
    panel_box.append(info_frame);

    set_commands();
    set_key_controller();
    set_mouse_controller();
    set_infos();
    set_drawing();
    set_jeu(file_name);
}
void My_window::set_commands()
{
    command_frame.set_child(command_box);
    for (auto &button : buttons)
    {
        command_box.append(button);
        button.set_margin(1);
    }
    for (auto &check : checks)
    {
        command_box.append(check);
        check.set_margin(1);
    }

    buttons[B_EXIT].signal_clicked().connect(sigc::mem_fun(*this,
                                                      &My_window::exit_clicked));
    buttons[B_OPEN].signal_clicked().connect(sigc::mem_fun(*this,
                                                      &My_window::open_clicked));
    buttons[B_SAVE].signal_clicked().connect(sigc::mem_fun(*this,
                                                      &My_window::save_clicked));
    buttons[B_RESTART].signal_clicked().connect(sigc::mem_fun(*this,
                                                      &My_window::restart_clicked));
    buttons[B_START].signal_clicked().connect(sigc::mem_fun(*this,
                                                      &My_window::start_clicked));
    buttons[B_STEP].signal_clicked().connect(sigc::mem_fun(*this,
                                                      &My_window::step_clicked));
    checks[0].set_active(true);
    checks[0].set_group(checks[1]);
    checks[0].signal_toggled().connect(sigc::mem_fun(*this,
                                                     &My_window::build_clicked));
    checks[1].signal_toggled().connect(sigc::mem_fun(*this,
                                                     &My_window::guide_clicked));
}

void My_window::exit_clicked()
{
    hide();
}
void My_window::open_clicked()
{
    auto dialog = new Gtk::FileChooserDialog("Choose a text file",
                                             Gtk::FileChooserDialog::Action::OPEN);
    set_dialog(dialog);
}
void My_window::save_clicked()
{
    auto dialog = new Gtk::FileChooserDialog("Choose a text file",
                                             Gtk::FileChooserDialog::Action::SAVE);
    set_dialog(dialog);

}
void My_window::restart_clicked()
{
    //initialise the game from the last loaded file
    if (_jeu->restart())
    {
        _jeu->clear();
        _jeu->set_jeu(previous_file_name);
        drawing.queue_draw(); // Refresh the drawing area
        update_infos();
        activated = false;
        buttons[B_EXIT].set_sensitive(true);
        buttons[B_OPEN].set_sensitive(true);
        buttons[B_SAVE].set_sensitive(true);
        buttons[B_RESTART].set_sensitive(true);
        buttons[B_START].set_label("start");
        buttons[B_STEP].set_sensitive(true);
    }
    else
    {
        cout << "Error: unable to restart the game" << endl;
    }
}
void My_window::start_clicked()
{
    if (activated) // Game is running, so pause it
    {
        loop_conn.disconnect();
        activated = false;
        buttons[B_EXIT].set_sensitive(true);
        buttons[B_OPEN].set_sensitive(true);
        buttons[B_SAVE].set_sensitive(true);
        buttons[B_RESTART].set_sensitive(true);
        buttons[B_START].set_label("start");
        buttons[B_STEP].set_sensitive(true);
    }
    else if (_jeu->getStatus() == ONGOING) // Game can be started
    {
        loop_conn = 
        Glib::signal_timeout().connect(sigc::mem_fun(*this, &My_window::loop), 32); 
            // 100 ms interval
        activated = true;
        buttons[B_EXIT].set_sensitive(false);
        buttons[B_OPEN].set_sensitive(false);
        buttons[B_SAVE].set_sensitive(false);
        buttons[B_RESTART].set_sensitive(false);
        buttons[B_START].set_label("stop");
        buttons[B_STEP].set_sensitive(false);
    }
}

void My_window::step_clicked()
{
    if (_jeu->getStatus() == ONGOING) // Only update if the game is ongoing
    {
        _jeu->update();
        update_infos();      // Update the info labels (score, particle count, etc.)
        drawing.queue_draw(); // Refresh the drawing area

        // Check if the game has ended after the update
        if (_jeu->getStatus() != ONGOING)
        {
            activated = false; // Ensure the game remains paused
            buttons[B_EXIT].set_sensitive(true);
            buttons[B_OPEN].set_sensitive(true);
            buttons[B_SAVE].set_sensitive(false); // Disable save if game is over
            buttons[B_RESTART].set_sensitive(true);
            buttons[B_START].set_label("start");
            buttons[B_START].set_sensitive(false); // Disable start if game is over
            buttons[B_STEP].set_sensitive(false); // Disable step if game is over
            checks[0].set_active(true);
            checks[0].set_sensitive(false);
            checks[1].set_sensitive(false);

        }
    }
}

void My_window::build_clicked()
{
    _jeu->set_mode(CONSTRUCTION);
}
void My_window::guide_clicked()
{
    _jeu->set_mode(GUIDAGE);
}
void My_window::set_key_controller()
{
    auto contr = Gtk::EventControllerKey::create();
    contr->signal_key_pressed().connect(sigc::mem_fun(*this, &My_window::key_pressed),
                                        false);
    add_controller(contr);
}
bool My_window::key_pressed(guint keyval, guint keycode, Gdk::ModifierType state)
{

    switch (keyval)
    {
    case '1':
        this->step_clicked();
        return true;
    case 's':
        this->start_clicked();
        return true;
    case 'r':
        this->restart_clicked();
        return true;
    default:
        return false;
    }
    return false;
}

void My_window::set_dialog(Gtk::FileChooserDialog *dialog)
{
    dialog->set_modal(true);
    dialog->set_transient_for(*this);
    dialog->set_select_multiple(false);
    dialog->signal_response().connect(sigc::bind(
        sigc::mem_fun(*this, &My_window::dialog_response), dialog));

    dialog->add_button("_Cancel", CANCEL);
    switch (dialog->get_action())
    {
    case Gtk::FileChooserDialog::Action::OPEN:
        dialog->add_button("_Open", OPEN);
        break;
    case Gtk::FileChooserDialog::Action::SAVE:
        dialog->add_button("_Save", SAVE);
        break;
    default:
        break;
    }

    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("Text files");
    filter_text->add_pattern("*.txt");
    dialog->add_filter(filter_text);

    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog->add_filter(filter_any);

    dialog->show();
}

void My_window::dialog_response(int response, Gtk::FileChooserDialog *dialog)
{
    string file_name = "";
    if (dialog->get_file())
    {
        file_name = dialog->get_file()->get_path();
        std::cout << "Selected file: " << file_name << std::endl;
        if (file_name.size() < 4 or file_name.substr(file_name.size() - 4) != ".txt")
        {
            std::cout << "File does not end with .txt" << std::endl;
            file_name = "";
        }
    }
    else
    {
        std::cout << "No file selected or get_file() failed" << std::endl;
    }

    switch (response)
    {
    case CANCEL:
        std::cout << "Dialog cancelled" << std::endl;
        dialog->hide();
        break;
    case OPEN:
        if (file_name != "")
        {
            std::cout << "Attempting to load file: " << file_name << std::endl;
            // Reset the game state
            _jeu->clear();
            activated = false; // Stop any running simulation
            buttons[B_EXIT].set_sensitive(true);
            buttons[B_OPEN].set_sensitive(true);
            buttons[B_SAVE].set_sensitive(true);
            buttons[B_RESTART].set_sensitive(true);
            buttons[B_START].set_label("start");
            buttons[B_START].set_sensitive(true);
            buttons[B_STEP].set_sensitive(true);
            checks[0].set_sensitive(true);
            checks[1].set_sensitive(true);

            // Load the new file
            if (_jeu->set_jeu(file_name))
            {
                std::cout << "File loaded successfully" << std::endl;
                _jeu->setStatus(ONGOING); // Set the game status to ongoing
                this->previous_file_name = file_name;
                // Update mode based on the loaded file
                switch (_jeu->getMode() == "CONSTRUCTION" ? CONSTRUCTION : GUIDAGE)
                {
                case CONSTRUCTION:
                    checks[0].set_active(true);
                    checks[1].set_active(false);
                    break;
                case GUIDAGE:
                    checks[1].set_active(true);
                    checks[0].set_active(false);
                    break;
                default:
                    checks[0].set_active(true);
                    checks[1].set_active(false);
                    break;
                }
            }
            else
            {
                std::cout << "Failed to load file: " << file_name << std::endl;
                _jeu->setStatus(ERROR); // Reset the game state
                _jeu->clear();
                // Disable buttons since the game state cleared and no file is loaded
                buttons[B_SAVE].set_sensitive(false);
                buttons[B_START].set_sensitive(false);
                buttons[B_STEP].set_sensitive(false);
                buttons[B_RESTART].set_sensitive(false);
                checks[0].set_active(true);
                checks[0].set_sensitive(false);
                checks[1].set_sensitive(false);
            }
            update_infos();      // Update info labels
            drawing.queue_draw(); // Refresh the display
            dialog->hide();
        }
        else
        {
            std::cout << "No valid file selected for OPEN action" << std::endl;
            dialog->hide();
        }
        break;
    case SAVE:
        if (file_name != "")
        {
            std::cout << "Saving to file: " << file_name << std::endl;
            _jeu->save(file_name);
            dialog->hide();
        }
        else
        {
            std::cout << "No valid file selected for SAVE action" << std::endl;
            dialog->hide();
        }
        break;
    default:
        std::cout << "Unknown dialog response: " << response << std::endl;
        dialog->hide();
        break;
    }
    delete dialog; // Prevent memory leak
}

bool My_window::loop()
{
    if (activated)
    {
        update();
        return true; // Continue the timer
    }
    return false; // Stop the timer
}

void My_window::update()
{
    _jeu->update();
    update_infos();
    drawing.queue_draw(); // Refresh the display
    counter += 1;

    if (_jeu->getStatus() != ONGOING)
    {
        loop_conn.disconnect(); // Stop the timer
        activated = false;
        buttons[B_EXIT].set_sensitive(true);
        buttons[B_OPEN].set_sensitive(true);
        buttons[B_SAVE].set_sensitive(false); // Disable save if game is over
        buttons[B_RESTART].set_sensitive(true);
        buttons[B_START].set_label("start");
        buttons[B_START].set_sensitive(false); // Disable start if game is over
        buttons[B_STEP].set_sensitive(false); // Disable step if game is over
        checks[0].set_active(true);
        checks[0].set_sensitive(false);
        checks[1].set_sensitive(false);
    }
}

void My_window::set_infos()
{
    info_frame.set_child(info_grid);
    info_grid.set_column_homogeneous(true);
    for (size_t i(0); i < info_text.size(); ++i)
    {
        info_grid.attach(info_text[i], 0, i, 1, 1);
        info_grid.attach(info_value[i], 1, i, 1, 1);
        info_text[i].set_halign(Gtk::Align::START);
        info_value[i].set_halign(Gtk::Align::END);
        info_text[i].set_margin(3);
        info_value[i].set_margin(3);
    }
}
void My_window::update_infos()
{
    info_value[0].set_text(to_string(_jeu->get_score()));
    info_value[1].set_text(to_string(_jeu->get_particle_count()));
    info_value[2].set_text(to_string(_jeu->get_faiseur_count()));
    info_value[3].set_text(to_string(_jeu->get_articulation_count()));
}

void My_window::set_drawing()
{
    drawing.set_content_width(taille_dessin);
    drawing.set_content_height(taille_dessin);
    drawing.set_expand();
    drawing.set_draw_func(sigc::mem_fun(*this, &My_window::on_draw));
}
void My_window::on_draw(const Cairo::RefPtr<Cairo::Context> &cr,
                        int width, int height)
{

    graphic_set_context(cr);
    double side(min(width, height));
    cr->translate(width / 2, height / 2);
    cr->scale(side / (2 * r_max), -side / (2 * r_max));

	cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->paint();

    if (_jeu->getStatus() != -1) 
    {
        _jeu->draw(); 
    }
}

void My_window::set_mouse_controller()
{
    auto left_click = Gtk::GestureClick::create();
    auto right_click = Gtk::GestureClick::create();
    auto move = Gtk::EventControllerMotion::create();

    left_click->set_button(GDK_BUTTON_PRIMARY);
    right_click->set_button(GDK_BUTTON_SECONDARY);

    left_click->signal_pressed().connect(
        sigc::mem_fun(*this, &My_window::on_drawing_left_click));
    right_click->signal_pressed().connect(
        sigc::mem_fun(*this, &My_window::on_drawing_right_click));
    move->signal_motion().connect(
        sigc::mem_fun(*this, &My_window::on_drawing_move));

    drawing.add_controller(left_click);
    drawing.add_controller(right_click);
    drawing.add_controller(move);
}

// cette fonction convertit l'entrée pos contenant les coordonnées (x,y) de la souris 
// dans l'espace GTKmm vers l'espace du Modèle => sortie de la fonction.
S2d My_window::scaled(S2d const &pos) const
{
    int width = drawing.get_width();
    int height = drawing.get_height();
    double ratio((2 * r_max) / min(width, height));
    return {ratio * (-width / 2 + pos.x),
            ratio * (height / 2 - pos.y)};
}

void My_window::on_drawing_left_click(int n_press, double x, double y) {
    S2d pos = scaled({x, y}); // Convert to model coordinates
    
    if (!activated && (_jeu->getStatus() == ONGOING)) { // Game is paused and ongoing
        checks[0].set_active(true); // Set to construction mode
        build_clicked();
        checks[1].set_active(false);
        
        // Attempt to construct the chain at the clicked position
        _jeu->handle_left_click(pos);
        _jeu->update();
        update_infos();
        drawing.queue_draw();
    }
}

void My_window::on_drawing_right_click(int n_press, double x, double y) {
    S2d pos = scaled({x, y}); // Convert to model coordinates
    
    if (!activated && (_jeu->getStatus() == ONGOING)) { // Game is paused and ongoing
        checks[1].set_active(true); // Set to guidance mode
        guide_clicked();
        checks[0].set_active(false);
        _jeu->handle_right_click(pos);
        _jeu->update();
        update_infos();
        drawing.queue_draw();
    }
}

void My_window::on_drawing_move(double x, double y) {
    S2d pos = scaled({x, y}); // Convert to model coordinates
    
    // Pass mouse position to the game to update capture region and goals
    _jeu->handle_mouse_move(pos);
    drawing.queue_draw(); // Update the display
}


void My_window::set_jeu(string file_name)
{
    if (!_jeu->set_jeu(file_name)) // Failed to read file
    {
        std::cout << "Error reading file: " << file_name << std::endl;
        _jeu->clear();
        buttons[B_SAVE].set_sensitive(false); 
        buttons[B_START].set_sensitive(false); 
        buttons[B_STEP].set_sensitive(false); 
        buttons[B_RESTART].set_sensitive(true); 
        checks[0].set_active(true);
        checks[0].set_sensitive(false); 
        checks[1].set_sensitive(false);
    }
    else 
    {
        previous_file_name = file_name; // Store the last successful file
        buttons[B_SAVE].set_sensitive(true); // Enable save
        buttons[B_START].set_sensitive(true); // Enable start
        buttons[B_STEP].set_sensitive(true); // Enable step
        buttons[B_RESTART].set_sensitive(true); // Enable restart
        checks[0].set_sensitive(true); // Enable Construction
        checks[1].set_sensitive(true); // Enable Guidage
        switch (_jeu->getStatus())
        {
        case CONSTRUCTION:
            checks[0].set_active(true);
            checks[1].set_active(false);
            break;
        case GUIDAGE:
            checks[1].set_active(true);
            checks[0].set_active(false);
            break;
        default:
            checks[0].set_active(true); 
            checks[1].set_active(false);
            break;
        }
    }
    update_infos(); 
    drawing.queue_draw(); 
}

int start_gui_application(int argc, char* argv[], 
    const std::string& filename, std::unique_ptr<Jeu> jeu) {
    try {
        // Create the GTK application
        auto app = Gtk::Application::create("org.example.app");
        
        // Connect to the activate signal
        app->signal_activate().connect([&app, &filename, &jeu]() {
            try {
                std::cout << "Creating window..." << std::endl;
                // Create the main window, passing ownership of jeu
                auto window = new My_window(filename, jeu.release());
                std::cout << "Window created" << std::endl;
                
                // Add window to the application and show it
                app->add_window(*window);
                window->show();
                
                // Make sure window gets deleted when closed
                window->signal_hide().connect([window]() {
                    delete window;
                });
            } catch (const std::exception& e) {
                std::cerr << "Exception when creating window: " << e.what()
                 << std::endl;
            } catch (...) {
                std::cerr << "Unknown exception occurred when creating window" 
                << std::endl;
            }
        });
        
        // Run the application
        std::cout << "Starting application..." << std::endl;
        return app->run(1, argv);  // Use argc=1 to avoid file handling issues
    } catch (const std::exception& e) {
        std::cerr << "Exception in GUI application: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception in GUI application" << std::endl;
        return 1;
    }
}
