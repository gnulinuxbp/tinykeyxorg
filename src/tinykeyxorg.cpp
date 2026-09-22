#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/fl_ask.H>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

static const char *CONF_DIR      = "/usr/local/share/X11/xorg.conf.d";
static const char *CONF_FILE     = "/usr/local/share/X11/xorg.conf.d/10-keyboard.conf";
static const char *FILETOOL      = "/opt/.filetool.lst";
static const char *FILETOOL_REL  = "usr/local/share/X11/xorg.conf.d/10-keyboard.conf";

static const char *DEFAULT_MODEL  = "pc105";
static const char *DEFAULT_LAYOUT = "es";

static const char *SCRIPT_PATH = "/tmp/tinykeyxorg_apply.sh";
static const char *LOG_PATH    = "/tmp/tinykeyxorg_apply.log";

static const char *LAYOUT_LABELS[] = {
    "Spanish (Spain) - es",
    "Spanish (Latin America) - latam",
    "English (US) - us",
    "English (UK) - gb",
    "French - fr",
    "German - de",
    "Italian - it",
    "Portuguese - pt",
    "Portuguese (Brazil) - br",
    "Other (type manually)"
};

static const char *LAYOUT_CODES[] = {
    "es", "latam", "us", "gb", "fr", "de", "it", "pt", "br"
};

static const int LAYOUT_COUNT = 9;
static const int LAYOUT_OTHER_INDEX = 9;

static Fl_Input        *inModel     = NULL;
static Fl_Choice        *layoutChoice = NULL;
static Fl_Input        *inLayout    = NULL;
static Fl_Input        *inVariant   = NULL;
static Fl_Input        *inOptions   = NULL;
static Fl_Text_Display *outLog      = NULL;
static Fl_Text_Buffer  *outBuf      = NULL;
static Fl_Button       *btnApply    = NULL;

static std::string shQuote(const std::string &s) {
    std::string out = "'";
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '\'') {
            out += "'\\''";
        } else {
            out += c;
        }
    }
    out += "'";
    return out;
}

static void logLine(const char *fmt, ...) {
    char buf[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    outBuf->append(buf);
    outBuf->append("\n");
    outLog->insert_position(outBuf->length());
    outLog->show_insert_position();
}

static std::string readFile(const char *path) {
    std::string content;
    FILE *f = fopen(path, "r");
    if (!f) return content;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        content.append(buf, n);
    }
    fclose(f);
    return content;
}

static bool g_pwdOk = false;
static std::string g_pwdValue;
static Fl_Secret_Input *pwdField = NULL;

static void pwd_ok_cb(Fl_Widget *w, void *data) {
    (void)w;
    g_pwdOk = true;
    g_pwdValue = pwdField->value() ? pwdField->value() : "";
    Fl_Window *win = (Fl_Window *)data;
    win->hide();
}

static void pwd_cancel_cb(Fl_Widget *w, void *data) {
    (void)w;
    g_pwdOk = false;
    g_pwdValue.clear();
    Fl_Window *win = (Fl_Window *)data;
    win->hide();
}

static bool askSudoPassword(std::string &out) {
    g_pwdOk = false;
    g_pwdValue.clear();

    Fl_Window win(360, 130, "Administrator privileges required");
    Fl_Box label(10, 10, 340, 30, "Enter your sudo password to continue:");
    label.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    Fl_Secret_Input pwd(120, 45, 220, 25, "Password:");
    pwdField = &pwd;
    pwd.align(FL_ALIGN_LEFT);

    Fl_Button okBtn(160, 85, 80, 30, "OK");
    okBtn.callback(pwd_ok_cb, &win);

    Fl_Button cancelBtn(250, 85, 90, 30, "Cancel");
    cancelBtn.callback(pwd_cancel_cb, &win);

    win.set_modal();
    win.end();
    win.show();
    pwd.take_focus();

    while (win.shown()) {
        Fl::wait();
    }

    out = g_pwdValue;
    return g_pwdOk;
}

static void layout_choice_cb(Fl_Widget *w, void *data) {
    (void)data;
    Fl_Choice *ch = (Fl_Choice *)w;
    int idx = ch->value();

    if (idx == LAYOUT_OTHER_INDEX) {
        inLayout->value("");
        inLayout->activate();
        inLayout->take_focus();
    } else if (idx >= 0 && idx < LAYOUT_COUNT) {
        inLayout->value(LAYOUT_CODES[idx]);
        inLayout->deactivate();
    }
}

static std::string buildScript(const std::string &model,
                                const std::string &layout,
                                const std::string &variant,
                                const std::string &options) {
    std::string s;
    s += "#!/bin/sh\n";
    s += "set -u\n\n";

    s += "MODEL=" + shQuote(model) + "\n";
    s += "LAYOUT=" + shQuote(layout) + "\n";
    s += "VARIANT=" + shQuote(variant) + "\n";
    s += "OPTIONS=" + shQuote(options) + "\n\n";

    s += "CONF_DIR=" + shQuote(CONF_DIR) + "\n";
    s += "CONF_FILE=" + shQuote(CONF_FILE) + "\n";
    s += "FILETOOL=" + shQuote(FILETOOL) + "\n";
    s += "FILETOOL_REL=" + shQuote(FILETOOL_REL) + "\n\n";

    s += "if [ -n \"$VARIANT\" ]; then\n"
         "    VARIANT_LINE=\"    Option \\\"XkbVariant\\\" \\\"$VARIANT\\\"\"\n"
         "else\n"
         "    VARIANT_LINE=\"\"\n"
         "fi\n\n";

    s += "if [ -n \"$OPTIONS\" ]; then\n"
         "    OPTIONS_LINE=\"    Option \\\"XkbOptions\\\" \\\"$OPTIONS\\\"\"\n"
         "else\n"
         "    OPTIONS_LINE=\"\"\n"
         "fi\n\n";

    s += "echo \"Model   : $MODEL\"\n";
    s += "echo \"Layout  : $LAYOUT\"\n";
    s += "if [ -n \"$VARIANT\" ]; then echo \"Variant : $VARIANT\"; "
         "else echo \"Variant : (none)\"; fi\n";
    s += "if [ -n \"$OPTIONS\" ]; then echo \"Options : $OPTIONS\"; "
         "else echo \"Options : (none)\"; fi\n";
    s += "echo\n\n";

    s += "if [ ! -d \"$CONF_DIR\" ]; then\n"
         "    echo \"Creating directory: $CONF_DIR\"\n"
         "    mkdir -p \"$CONF_DIR\" || { echo \"ERROR: could not create $CONF_DIR\"; exit 1; }\n"
         "fi\n\n";

    s += "if [ -f \"$CONF_FILE\" ]; then\n"
         "    BACKUP_FILE=\"${CONF_FILE}.bak\"\n"
         "    echo \"Already exists: $CONF_FILE\"\n"
         "    if [ -e \"$BACKUP_FILE\" ]; then\n"
         "        BACKUP_FILE=\"${CONF_FILE}.bak.$(date +%Y%m%d%H%M%S)\"\n"
         "    fi\n"
         "    echo \"Creating backup: $BACKUP_FILE\"\n"
         "    cp \"$CONF_FILE\" \"$BACKUP_FILE\" || { echo \"ERROR: could not create backup\"; exit 1; }\n"
         "fi\n\n";

    s += "echo \"Writing Xorg configuration...\"\n";
    s += "cat > \"$CONF_FILE\" <<EOF\n"
         "Section \"InputClass\"\n"
         "    Identifier \"Keyboard Defaults\"\n"
         "    MatchIsKeyboard \"yes\"\n"
         "    Option \"XkbModel\" \"$MODEL\"\n"
         "    Option \"XkbLayout\" \"$LAYOUT\"\n"
         "$VARIANT_LINE\n"
         "$OPTIONS_LINE\n"
         "EndSection\n"
         "EOF\n\n";

    s += "if [ $? -ne 0 ]; then echo \"ERROR: could not write $CONF_FILE\"; exit 1; fi\n\n";

    s += "chmod 644 \"$CONF_FILE\"\n\n";

    s += "echo \"Configuring persistence...\"\n";
    s += "touch \"$FILETOOL\"\n";
    s += "if grep -Fxq \"$FILETOOL_REL\" \"$FILETOOL\"; then\n"
         "    echo \"Already listed in: $FILETOOL\"\n"
         "else\n"
         "    echo \"$FILETOOL_REL\" >> \"$FILETOOL\" || { echo \"ERROR: could not modify $FILETOOL\"; exit 1; }\n"
         "    echo \"Added to: $FILETOOL\"\n"
         "fi\n\n";

    s += "echo \"Running Tiny Core backup (filetool.sh -b)...\"\n";
    s += "filetool.sh -b\n";
    s += "if [ $? -ne 0 ]; then\n"
         "    echo \"WARNING: filetool.sh finished with an error.\"\n"
         "    echo \"The configuration was created, but persistence could not be confirmed.\"\n"
         "    exit 1\n"
         "fi\n\n";

    s += "echo\n";
    s += "echo \"CONFIGURATION COMPLETE\"\n";
    s += "echo \"File created : $CONF_FILE\"\n";
    s += "echo \"Persistence  : $FILETOOL\"\n";
    s += "exit 0\n";

    return s;
}

static int runScript(bool needSudo, const std::string &password) {
    remove(LOG_PATH);

    std::string cmd;
    if (needSudo) {
        cmd = std::string("sudo -S -p '' /bin/sh ") + SCRIPT_PATH +
              " > " + LOG_PATH + " 2>&1";
    } else {
        cmd = std::string("/bin/sh ") + SCRIPT_PATH +
              " > " + LOG_PATH + " 2>&1";
    }

    int status;
    if (needSudo) {
        FILE *p = popen(cmd.c_str(), "w");
        if (!p) {
            logLine("ERROR: could not invoke sudo.");
            return -1;
        }
        fprintf(p, "%s\n", password.c_str());
        fflush(p);
        status = pclose(p);
    } else {
        status = system(cmd.c_str());
    }

    std::string output = readFile(LOG_PATH);
    if (!output.empty()) {
        outBuf->append(output.c_str());
        outLog->insert_position(outBuf->length());
        outLog->show_insert_position();
    }

    if (status == -1) {
        return -1;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

static void apply_cb(Fl_Widget *w, void *data) {
    (void)w; (void)data;

    outBuf->text("");

    std::string model   = inModel->value()   ? inModel->value()   : "";
    std::string layout  = inLayout->value()  ? inLayout->value()  : "";
    std::string variant = inVariant->value() ? inVariant->value() : "";
    std::string options = inOptions->value() ? inOptions->value() : "";

    if (model.empty())  model  = DEFAULT_MODEL;
    if (layout.empty()) layout = DEFAULT_LAYOUT;

    inModel->value(model.c_str());
    if (layoutChoice->value() == LAYOUT_OTHER_INDEX) {
        inLayout->value(layout.c_str());
    }

    logLine("==============================================");
    logLine(" TinyKeyXorg - Xorg keyboard configuration");
    logLine("==============================================");

    std::string script = buildScript(model, layout, variant, options);

    FILE *f = fopen(SCRIPT_PATH, "w");
    if (!f) {
        fl_alert("Could not create temporary script at %s", SCRIPT_PATH);
        return;
    }
    fwrite(script.data(), 1, script.size(), f);
    fclose(f);
    chmod(SCRIPT_PATH, 0700);

    bool needSudo = (geteuid() != 0);
    std::string password;

    if (needSudo) {
        logLine("Administrator privileges are required.");
        if (!askSudoPassword(password)) {
            logLine("Operation cancelled by the user.");
            return;
        }
    }

    btnApply->deactivate();
    Fl::check();

    int rc = runScript(needSudo, password);

    btnApply->activate();

    logLine("----------------------------------------------");
    if (rc == 0) {
        logLine("Process finished successfully.");
        logLine("It is recommended to restart your system for the changes to take full effect.");
        fl_message("Configuration applied successfully.\n\nIt is recommended to restart your system for the changes to take full effect.");
    } else if (rc == -1) {
        logLine("The process could not be executed (wrong password or sudo unavailable?).");
        fl_alert("An error occurred while running the process.\nCheck the log in the window.");
    } else {
        logLine("The process finished with exit code %d.", rc);
        fl_alert("The process finished with errors (code %d).\nCheck the log in the window.", rc);
    }

    remove(SCRIPT_PATH);
}

static void quit_cb(Fl_Widget *w, void *data) {
    (void)w; (void)data;
    exit(0);
}

int main(int argc, char **argv) {
    Fl_Window win(520, 460, "TinyKeyXorg");

    int labelW = 140;
    int fieldX = 10 + labelW;
    int fieldW = 520 - fieldX - 20;
    int y = 15;

    Fl_Box title(10, y, 500, 25, "TinyKeyXorg - Xorg keyboard configuration");
    title.labelfont(FL_BOLD);
    title.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    y += 40;

    inModel = new Fl_Input(fieldX, y, fieldW, 25, "Keyboard model:");
    inModel->align(FL_ALIGN_LEFT);
    inModel->value(DEFAULT_MODEL);
    y += 35;

    int choiceW = 190;
    int manualX = fieldX + choiceW + 10;
    int manualW = fieldW - choiceW - 10;

    layoutChoice = new Fl_Choice(fieldX, y, choiceW, 25, "Layout:");
    layoutChoice->align(FL_ALIGN_LEFT);
    for (int i = 0; i < LAYOUT_COUNT; ++i) {
        layoutChoice->add(LAYOUT_LABELS[i]);
    }
    layoutChoice->add(LAYOUT_LABELS[LAYOUT_OTHER_INDEX]);
    layoutChoice->value(0);
    layoutChoice->callback(layout_choice_cb);

    inLayout = new Fl_Input(manualX, y, manualW, 25);
    inLayout->value(DEFAULT_LAYOUT);
    inLayout->deactivate();
    y += 35;

    inVariant = new Fl_Input(fieldX, y, fieldW, 25, "Variant (optional):");
    inVariant->align(FL_ALIGN_LEFT);
    y += 35;

    inOptions = new Fl_Input(fieldX, y, fieldW, 25, "XKB options (optional):");
    inOptions->align(FL_ALIGN_LEFT);
    y += 45;

    btnApply = new Fl_Button(10, y, 200, 32, "Apply configuration");
    btnApply->callback(apply_cb);

    Fl_Button *btnQuit = new Fl_Button(220, y, 100, 32, "Quit");
    btnQuit->callback(quit_cb);
    y += 45;

    outBuf = new Fl_Text_Buffer();
    outLog = new Fl_Text_Display(10, y, 500, 460 - y - 10);
    outLog->buffer(outBuf);
    outLog->textfont(FL_COURIER);
    outLog->textsize(12);

    win.resizable(outLog);
    win.end();
    win.show(argc, argv);

    return Fl::run();
}
