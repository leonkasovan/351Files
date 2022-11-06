#include "mainWindow.h"
#include <iostream>
#include <sstream>
#include <climits>
#include "def.h"
#include "sdlutils.h"
#include "fileUtils.h"
#include "dialog.h"
#include "textInput.h"
#include "textViewer.h"
#include "textEditor.h"
#include "imageViewer.h"
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// Destructor
MainWindow::~MainWindow(void) {}

//------------------------------------------------------------------------------

// Constructor
MainWindow::MainWindow(const std::string & p_title):
    IWindow(true, p_title),
    m_scrollFileNameActive(false),
    m_scrollFileNameX(0),
    m_scrollFileNameTimer(0),
    m_scrollFileNameDir(1) {
        // List files
        if (!m_fileLister.list(m_title)) {
            // Path is wrong, fallback to '/'
            m_title = "/";
            m_fileLister.list(m_title);
        }
        // Number of items
        m_nbItems = m_fileLister.getNbElements();
        // Adjust scrollbar
        adjustScrollbar();
        INHIBIT(std::cout << "Path: " << m_title << " (" << m_nbItems << ") items\n";)
    }

//------------------------------------------------------------------------------

// Draw window
void MainWindow::render(const bool p_focus) {
    // Clear screen
    SDL_SetRenderDrawColor(g_renderer, COLOR_BODY_BG, 255);
    SDL_RenderClear(g_renderer);

    // Render title background
    SDL_SetRenderDrawColor(g_renderer, COLOR_TITLE_BG, 255);
    SDL_Rect rect {
        0,
        0,
        SCREEN_WIDTH,
        LINE_HEIGHT
    };
    SDL_RenderFillRect(g_renderer, & rect);

    // Render title
    int l_y = LINE_HEIGHT / 2;
    SDLUtils::renderTexture(g_iconFloppy, MARGIN_X, l_y, SDLUtils::T_ALIGN_LEFT, SDLUtils::T_ALIGN_MIDDLE);
    SDLUtils::renderText(m_title, g_font, MARGIN_X + ICON_SIZE + MARGIN_X, l_y, {
        COLOR_TEXT_NORMAL
    }, {
        COLOR_TITLE_BG
    }, SDLUtils::T_ALIGN_LEFT, SDLUtils::T_ALIGN_MIDDLE);

    // Render cursor
    if (p_focus)
        SDL_SetRenderDrawColor(g_renderer, COLOR_CURSOR_FOCUS, 255);
    else
        SDL_SetRenderDrawColor(g_renderer, COLOR_CURSOR_NO_FOCUS, 255);
    rect.x = 0;
    rect.y = LINE_HEIGHT + (m_cursor - m_camera.y) * LINE_HEIGHT;
    rect.w = SCREEN_WIDTH - m_scrollbar.w;
    rect.h = LINE_HEIGHT;
    SDL_RenderFillRect(g_renderer, & rect);

    // Render scrollbar
    if (m_scrollbar.h > 0)
        SDL_RenderFillRect(g_renderer, & m_scrollbar);

    // Render file list
    l_y += LINE_HEIGHT;
    SDL_Color l_fgColor = {
        COLOR_TEXT_NORMAL
    };
    SDL_Color l_bgColor = {
        COLOR_BODY_BG
    };
    int sizeW = 0, fileNameMaxWidth = 0, fileNameTextureWidth = 0;
    for (int l_i = m_camera.y; l_i < m_camera.y + m_nbVisibleLines && l_i < m_nbItems; ++l_i) {
        // Colors for the line
        if (m_fileLister[l_i].m_selected)
            l_fgColor = {
                COLOR_TEXT_SELECTED
            };
        else
            l_fgColor = {
                COLOR_TEXT_NORMAL
            };
        l_bgColor = getBackgroundColor(l_i, p_focus);

        // Icon
        if (m_fileLister[l_i].m_name == "..")
            SDLUtils::renderTexture(g_iconUp, MARGIN_X, l_y, SDLUtils::T_ALIGN_LEFT, SDLUtils::T_ALIGN_MIDDLE);
        else if (m_fileLister.isDirectory(l_i))
            SDLUtils::renderTexture(g_iconDir, MARGIN_X, l_y, SDLUtils::T_ALIGN_LEFT, SDLUtils::T_ALIGN_MIDDLE);
        else if (ImageViewer::extensionIsSupported(m_fileLister[l_i].m_ext))
            SDLUtils::renderTexture(g_iconImage, MARGIN_X, l_y, SDLUtils::T_ALIGN_LEFT, SDLUtils::T_ALIGN_MIDDLE);
        else if (m_fileLister[l_i].m_symlink)
            SDLUtils::renderTexture(g_iconCopy, MARGIN_X, l_y, SDLUtils::T_ALIGN_LEFT, SDLUtils::T_ALIGN_MIDDLE);
        else
			SDLUtils::renderTexture(g_iconFile, MARGIN_X, l_y, SDLUtils::T_ALIGN_LEFT, SDLUtils::T_ALIGN_MIDDLE);

        // File size
        if (m_fileLister[l_i].m_size == ULLONG_MAX)
            sizeW = 0;
        else
            sizeW = SDLUtils::renderText(FileUtils::formatSize(m_fileLister[l_i].m_size), g_font, SCREEN_WIDTH - m_scrollbar.w - MARGIN_X, l_y, l_fgColor, l_bgColor, SDLUtils::T_ALIGN_RIGHT, SDLUtils::T_ALIGN_MIDDLE);

        // File name
        fileNameMaxWidth = SCREEN_WIDTH - 4 * MARGIN_X - ICON_SIZE - m_scrollbar.w - sizeW;
        if (m_cursor == l_i) {
            if (m_scrollFileNameActive) {
                // Render file name with scrolling
                fileNameTextureWidth = SDLUtils::renderTextScrolling(m_fileLister[l_i].m_name, g_font, MARGIN_X + ICON_SIZE + MARGIN_X, l_y, l_fgColor, l_bgColor, SDLUtils::T_ALIGN_LEFT, SDLUtils::T_ALIGN_MIDDLE, fileNameMaxWidth, m_scrollFileNameX);
                --m_scrollFileNameTimer;
                if (m_scrollFileNameTimer <= 0) {
                    m_scrollFileNameX += m_scrollFileNameDir;
                    if (m_scrollFileNameX > fileNameTextureWidth - fileNameMaxWidth) {
                        m_scrollFileNameX = fileNameTextureWidth - fileNameMaxWidth;
                        m_scrollFileNameTimer = LONG_NAME_TIMER_FIRST * 2;
                        m_scrollFileNameDir = -1;
                    } else if (m_scrollFileNameX < 0) {
                        m_scrollFileNameX = 0;
                        m_scrollFileNameTimer = LONG_NAME_TIMER_FIRST * 2;
                        m_scrollFileNameDir = 1;
                    } else {
                        m_scrollFileNameTimer = LONG_NAME_TIMER;
                    }
                }
                g_hasChanged = true;
            } else {
                fileNameTextureWidth = SDLUtils::renderText(m_fileLister[l_i].m_name, g_font, MARGIN_X + ICON_SIZE + MARGIN_X, l_y, l_fgColor, l_bgColor, SDLUtils::T_ALIGN_LEFT, SDLUtils::T_ALIGN_MIDDLE, fileNameMaxWidth);
                // Activate scrolling if file name is too long
                if (!m_scrollFileNameActive && fileNameTextureWidth > fileNameMaxWidth) {
                    m_scrollFileNameActive = true;
                    m_scrollFileNameX = 0;
                    m_scrollFileNameTimer = LONG_NAME_TIMER_FIRST;
                    m_scrollFileNameDir = 1;
                    g_hasChanged = true;
                }
            }
        } else {
            SDLUtils::renderText(m_fileLister[l_i].m_name, g_font, MARGIN_X + ICON_SIZE + MARGIN_X, l_y, l_fgColor, l_bgColor, SDLUtils::T_ALIGN_LEFT, SDLUtils::T_ALIGN_MIDDLE, fileNameMaxWidth);
        }

        // Next line
        l_y += LINE_HEIGHT;
    }

}

//------------------------------------------------------------------------------

// Move cursor up / down
void MainWindow::moveCursorUp(const int p_step, bool p_loop) {
    IWindow::moveCursorUp(p_step, p_loop);
    adjustCamera();
    adjustScrollbarPosition();
    m_scrollFileNameActive = false;
}

void MainWindow::moveCursorDown(const int p_step, bool p_loop) {
    IWindow::moveCursorDown(p_step, p_loop);
    adjustCamera();
    adjustScrollbarPosition();
    m_scrollFileNameActive = false;
}

//------------------------------------------------------------------------------

// Key pressed
void MainWindow::keyPressed(const SDL_Event & event) {
    // Button Validate
    if (BUTTON_PRESSED_VALIDATE) {
        // Reset timer
        resetTimer();
        // Open directory or file
        if (m_fileLister.isDirectory(m_cursor))
            openHighlightedDir();
        else
            openHighlightedFile();
        m_scrollFileNameActive = false;
        return;
    }
    // Button Back
    if (BUTTON_PRESSED_BACK) {
        // Reset timer
        resetTimer();
        // If we're already at '/, do nothing
        if (m_title == "/")
            return;
        // Select and open ".."
        m_cursor = 0;
        openHighlightedDir();
        m_scrollFileNameActive = false;
        return;
    }
    // Button context menu
    if (BUTTON_PRESSED_MENU_CONTEXT) {
        // Reset timer
        resetTimer();
        // Open context menu
        openContextMenu();
        return;
    }
    // Button select
    if (BUTTON_PRESSED_SELECT) {
        // Reset timer
        resetTimer();
        // Select / unselect highlighted item
        selectHighlightedItem(true);
        return;
    }
	
	if (BUTTON_PRESSED_QUIT) {
        // Reset timer
        resetTimer();
		m_retVal = 0;
        return;
    }
}

//------------------------------------------------------------------------------

// Open highlighted dir
void MainWindow::openHighlightedDir(void) {
    // Build new path
    std::string l_newDir("");
    std::string l_oldDir("");
    if (m_fileLister[m_cursor].m_name == "..") {
        // New path = parent
        size_t l_pos = m_title.rfind('/');
        l_newDir = m_title.substr(0, l_pos);
        if (l_newDir.empty())
            l_newDir = "/";
        l_oldDir = m_title.substr(l_pos + 1);
    } else {
        // New path
        l_newDir = m_title + (m_title == "/" ? "" : "/") + m_fileLister[m_cursor].m_name;
    }

    // List the new path
    if (!m_fileLister.list(l_newDir)) {
        // An error occurred, stay at current dir
        m_fileLister.list(m_title);
        g_hasChanged = true;
        return;
    }

    // New path is OK
    m_title = l_newDir;
    m_nbItems = m_fileLister.getNbElements();
    // If it's a back movement, restore old highlighted dir
    if (!l_oldDir.empty())
        m_cursor = m_fileLister.searchDir(l_oldDir);
    else
        m_cursor = 0;
    // Adjust camera
    adjustCamera();
    // Adjust scrollbar
    adjustScrollbar();
    // New render
    g_hasChanged = true;
	// tambahkan chdir
	chdir(l_newDir.c_str());
    INHIBIT(std::cout << "Path: " << m_title << " (" << m_nbItems << ") items\n";)
}

//------------------------------------------------------------------------------

// Open highlighted dir
void MainWindow::openHighlightedFile(void) {
    // Case: file is a supported image
    if (ImageViewer::extensionIsSupported(m_fileLister[m_cursor].m_ext)) {
        ImageViewer imageViewer(m_title, & m_fileLister, m_cursor);
        imageViewer.execute();
        return;
    }

    // If the file is binary, show file attribute 
    std::string filePath = m_title + (m_title == "/" ? "" : "/") + m_fileLister[m_cursor].m_name;
    if (!(FileUtils::fileIsText(filePath) || m_fileLister[m_cursor].m_size == 0)) {
		std::string desc;
		char *p, *start;
		int len;
        Dialog l_dialog("File Properties:");
		std::string cmd = "file -b \"" + filePath + "\"";
		char result[2000];
		FILE *pipe = popen(cmd.c_str(), "r");
		if (pipe == NULL){
			std::cerr << "Error popen\n";
			return;
		}
		while (fgets(result, sizeof(result), pipe) != NULL);
		pclose(pipe);
		len = 0;
		start = p = result;
		while(*p){
			if (len > 30){
				if (*p == ' '){
					*p = 0;
					len = 0;
					desc = start;
					l_dialog.addLabel(desc);
					start = p+1;
				}
			}
			len++;
			p++;
		}
		p--;
		if (*p == '\n'){
			*p = 0;
			l_dialog.addLabel(start);
		}

		char tbuf[2000];
		struct stat sfile; //pointer to stat struct
		stat(filePath.c_str(), &sfile); //stat system call
		strftime(tbuf, 2000, "Last modification: %d-%m-%Y %H:%M:%S", localtime(&(sfile.st_mtime))); l_dialog.addLabel(tbuf);
		// strftime(tbuf, 2000, "Last access: %d-%m-%Y %H:%M:%S", localtime(&(sfile.st_atime))); l_dialog.addLabel(tbuf);
		l_dialog.addLabel(" ");
		if (m_fileLister[m_cursor].m_ext == "deb") { l_dialog.addOption("View contents", 1, g_iconFile); }
		if (m_fileLister[m_cursor].m_ext == "deb") { l_dialog.addOption("Install", 2, g_iconDisk); }
		if (m_fileLister[m_cursor].m_ext == "deb") { l_dialog.addOption("Extract", 3, g_iconNewDir); }
		
		if (m_fileLister[m_cursor].m_ext == "zip") { l_dialog.addOption("View contents", 4, g_iconFile); }
		if (m_fileLister[m_cursor].m_ext == "zip") { l_dialog.addOption("Extract", 5, g_iconNewDir); }
		
		if (m_fileLister[m_cursor].m_ext == "tar") { l_dialog.addOption("View contents", 6, g_iconFile); }
		if (m_fileLister[m_cursor].m_ext == "tar") { l_dialog.addOption("Extract", 7, g_iconNewDir); }
		
		if (m_fileLister[m_cursor].m_ext == "7z") { l_dialog.addOption("View contents", 8, g_iconFile); }
		if (m_fileLister[m_cursor].m_ext == "7z") { l_dialog.addOption("Extract", 9, g_iconNewDir); }
		
		if (m_fileLister[m_cursor].m_ext == "gz") { l_dialog.addOption("Extract and keep", 10, g_iconNewDir); }
		if (m_fileLister[m_cursor].m_ext == "gz") { l_dialog.addOption("Extract and delete", 11, g_iconNewDir); }
		
		if (m_fileLister[m_cursor].m_ext == "xz") { l_dialog.addOption("Extract and keep", 12, g_iconNewDir); }
		if (m_fileLister[m_cursor].m_ext == "xz") { l_dialog.addOption("Extract and delete", 13, g_iconNewDir); }
		
        l_dialog.addOption("Close", 0, g_iconCancel);
        int action = l_dialog.execute();
		
		if (action == 1){ // View deb contents
			std::string cmd = "ar x "+filePath+" data.tar.xz && unxz data.tar.xz && tar tf data.tar >contents.deb.txt && rm data.tar";
			int rc = system(cmd.c_str());
			if (!rc){
				TextViewer textViewer("contents.deb.txt");
				textViewer.execute();
				FileUtils::runCommand("rm", "contents.deb.txt");
			}else{
				Dialog l_dialog("Warning:");
				l_dialog.addLabel("Content can not be viewed.");
				l_dialog.addOption("Close", 1, g_iconCancel);
				l_dialog.execute();
			}
		}else if (action == 2){	//Install deb file into [bin][lib]
			std::string cmd = "cd /tmp && ar x "+filePath+" data.tar.xz && unxz data.tar.xz && mkdir -p tmp_tar && tar xf data.tar -Ctmp_tar && rm data.tar && cd tmp_tar" 
			+ " && if [[ -d usr/lib/aarch64-linux-gnu ]]; then rsync -a usr/lib/aarch64-linux-gnu/ /storage/usr/lib/; fi"
			+ " && if [[ -d usr/bin ]]; then rsync -a usr/bin/ /storage/usr/bin/; fi"
			+ " && if [[ -d lib/aarch64-linux-gnu ]]; then rsync -a lib/aarch64-linux-gnu/ /storage/lib/; fi"
			+ " && if [[ -d bin ]]; then rsync -a bin/ /storage/bin/; fi"
			+ " && cd .. && rm -r tmp_tar";
			int rc = system(cmd.c_str());
			if (rc){
				Dialog l_dialog("Error:");
				l_dialog.addLabel("Can't install deb file.");
				l_dialog.addOption("Close", 1, g_iconCancel);
				l_dialog.execute();
			}else{
				Dialog l_dialog("Info:");
				l_dialog.addLabel("Successfully install deb to [lib]/[bin]");
				l_dialog.addOption("Close", 1, g_iconSelect);
				l_dialog.execute();
			}
			chdir(m_title.c_str());
		}else if (action == 3){	// Extract deb file
			std::string cmd = "ar x "+filePath+" data.tar.xz && unxz data.tar.xz && tar xf data.tar && rm data.tar";
			int rc = system(cmd.c_str());
			if (rc){
				Dialog l_dialog("Error:");
				l_dialog.addLabel("Can't extract deb file.");
				l_dialog.addOption("Close", 1, g_iconCancel);
				l_dialog.execute();
			}
		}else if (action == 4){	// View zip contents
			std::string cmd = "unzip -l "+filePath+" >contents.zip.txt";
			int rc = system(cmd.c_str());
			if (!rc){
				TextViewer textViewer("contents.zip.txt");
				textViewer.execute();
				FileUtils::runCommand("rm", "contents.zip.txt");
			}else{
				Dialog l_dialog("Warning:");
				l_dialog.addLabel("Content can not be viewed.");
				l_dialog.addOption("Close", 1, g_iconCancel);
				l_dialog.execute();
			}
		}else if (action == 5){ //	Extract zip file
			FileUtils::runCommand("unzip", filePath);
		}else if (action == 6){	//	View tar contents
			std::string cmd = "tar tf "+filePath+" >contents.tar.txt";
			int rc = system(cmd.c_str());
			if (!rc){
				TextViewer textViewer("contents.tar.txt");
				textViewer.execute();
				FileUtils::runCommand("rm", "contents.tar.txt");
			}else{
				Dialog l_dialog("Warning:");
				l_dialog.addLabel("Content can not be viewed.");
				l_dialog.addOption("Close", 1, g_iconCancel);
				l_dialog.execute();
			}
		}else if (action == 7){	// Extract tar file
			FileUtils::runCommand("tar", "xf", filePath);
		}else if (action == 8){	// View 7z contents
			std::string cmd = "7zr l "+filePath+" >contents.7z.txt";
			int rc = system(cmd.c_str());
			if (!rc){
				TextViewer textViewer("contents.7z.txt");
				textViewer.execute();
				FileUtils::runCommand("rm", "contents.7z.txt");
			}else{
				Dialog l_dialog("Warning:");
				l_dialog.addLabel("Content can not be viewed.");
				l_dialog.addOption("Close", 1, g_iconCancel);
				l_dialog.execute();
			}
		}else if (action == 9){	// Extract 7z file
			FileUtils::runCommand("7zr", "x", filePath);
		}else if (action == 10){	// Extract and keep gz file
			FileUtils::runCommand("gzip", "-dk", filePath);
		}else if (action == 11){	// Extract and delete gz file
			FileUtils::runCommand("gzip", "-d", filePath);
		}else if (action == 10){	// Extract and keep xz file
			FileUtils::runCommand("unxz", "-dk", filePath);
		}else if (action == 11){	// Extract and delete xz file
			FileUtils::runCommand("unxz", filePath);
		}
		refresh();
        return;
    }

    // Dialog 'view as text' / 'edit as text'
    int action = -1; {
        Dialog l_dialog("Open:");
		char tbuf[2000];
		struct stat sfile; //pointer to stat struct
		stat(filePath.c_str(), &sfile); //stat system call
		strftime(tbuf, 2000, "Last modification: %d-%m-%Y %H:%M:%S", localtime(&(sfile.st_mtime))); l_dialog.addLabel(tbuf);
		// strftime(tbuf, 2000, "Last access: %d-%m-%Y %H:%M:%S", localtime(&(sfile.st_atime))); l_dialog.addLabel(tbuf);
		l_dialog.addLabel(" ");
        l_dialog.addOption("View as text", 1, g_iconFileText);
        l_dialog.addOption("Edit as text", 2, g_iconEdit);
		if (m_fileLister[m_cursor].m_ext == "lua") { l_dialog.addOption("Run with [lua]", 3, g_iconFile); }
		if (m_fileLister[m_cursor].m_ext == "lua") { l_dialog.addOption("Run with [luax]", 4, g_iconFile); }
        l_dialog.addOption("Cancel", 0, g_iconCancel);
        action = l_dialog.execute();
    }
    if (action < 1)
        return;

    // If the file is > 1M, ask for confirmation
    if (m_fileLister[m_cursor].m_size > 1024 * 1024) {
        Dialog l_dialog("Question:");
        l_dialog.addLabel("The file is big. Open anyway?");
        l_dialog.addOption("Yes", 0, g_iconSelect);
        l_dialog.addOption("No", 1, g_iconCancel);
        if (l_dialog.execute() != 0)
            return;
    }

    // View file as text
    if (action == 1) {
        TextViewer textViewer(filePath);
        textViewer.execute();
    }else if (action == 3) {
		system(("lua "+filePath+" >contents.lua.txt").c_str());
		TextViewer textViewer("contents.lua.txt");
		textViewer.execute();
		FileUtils::runCommand("rm", "contents.lua.txt");
    }else if (action == 4) {
        system(("luax "+filePath+" >contents.luax.txt").c_str());
		TextViewer textViewer("contents.luax.txt");
		textViewer.execute();
		FileUtils::runCommand("rm", "contents.luax.txt");
    }
    // Edit file as text
    else {
        TextEditor textEditor(filePath);
        textEditor.execute();
        refresh();
    }
}

//------------------------------------------------------------------------------

// Select highlighted item
void MainWindow::selectHighlightedItem(const bool p_step) {
    // Cannot select '..'
    if (m_fileLister[m_cursor].m_name == "..")
        return;
    // Select/unselect highlighted item
    m_fileLister[m_cursor].m_selected = !m_fileLister[m_cursor].m_selected;
    // Move 1 step if requested
    if (p_step)
        moveCursorDown(1, false);
    g_hasChanged = true;
}

//------------------------------------------------------------------------------

// Open context menu
void MainWindow::openContextMenu(void) {
    // If no file is selected, select current file
    auto nbSelected = m_fileLister.getNbSelected();
    bool autoUnselect = false;
    if (nbSelected == 0) {
        selectHighlightedItem(false);
        nbSelected = m_fileLister.getNbSelected();
        autoUnselect = true;
    }
    // Open dialog
    int result = -1; {
        std::ostringstream oss;
        oss << nbSelected << " selected";
        Dialog l_dialog(oss.str());
        if (nbSelected > 0) {
            l_dialog.addOption("Copy", 0, g_iconCopy);
            l_dialog.addOption("Cut", 1, g_iconCut);
        }
        if (m_clipboard.size() > 0)
            l_dialog.addOption("Paste", 2, g_iconPaste);
        if (nbSelected > 0)
            l_dialog.addOption("Delete", 3, g_iconTrash);
        if (nbSelected == 1)
            l_dialog.addOption("Rename", 9, g_iconEdit);
        if (m_fileLister.getNbSelected('d') > 0)
            l_dialog.addOption("Size", 4, g_iconDisk);
        l_dialog.addOption("Select all", 5, g_iconSelect);
        l_dialog.addOption("Select none", 6, g_iconNone);
        l_dialog.addOption("New directory", 7, g_iconNewDir);
        l_dialog.addOption("Quit", 8, g_iconQuit);
        result = l_dialog.execute();
    }
    switch (result) {
        // Copy
    case 0:
        m_fileLister.getSelectList(m_title, m_clipboard);
        m_clipboardOperation = 'c';
        INHIBIT(std::cout << m_clipboard.size() << " added to clipboard for copy\n";)
        break;
        // Move
    case 1:
        m_fileLister.getSelectList(m_title, m_clipboard);
        m_clipboardOperation = 'm';
        INHIBIT(std::cout << m_clipboard.size() << " added to clipboard for move\n";)
        break;
        // Paste
    case 2:
        FileUtils::copyOrMoveFiles(m_clipboardOperation, m_clipboard, m_title);
        refresh();
        break;
        // Delete
    case 3:
        m_fileLister.getSelectList(m_title, m_clipboard);
        FileUtils::removeFiles(m_clipboard);
        m_clipboard.clear();
        refresh();
        break;
        // Size
    case 4: {
        // Display a "please wait" message
        Dialog dialogPleaseWait("Info");
        dialogPleaseWait.addLabel("Please wait...");
        dialogPleaseWait.render(true);
        IWindow::renderPresent();
        m_fileLister.computeSelectedDirSize(m_title);
        g_hasChanged = true;
    }
    break;
    // Select all
    case 5:
        m_fileLister.setSelectedAll(true);
        autoUnselect = false;
        g_hasChanged = true;
        break;
        // Select none
    case 6:
        m_fileLister.setSelectedAll(false);
        g_hasChanged = true;
        break;
        // Quit
    case 8:
        m_retVal = 0;
        break;
        // New directory
    case 7: {
        TextInput textInput("New directory", g_iconNewDir);
        if (textInput.execute() != -2 && !textInput.getInputText().empty()) {
            FileUtils::makeDirectory(m_title + (m_title == "/" ? "" : "/") + textInput.getInputText());
            refresh();
        }
    }
    break;
    // Rename
    case 9: {
        std::string fileSrc = m_fileLister.getSelectFirst();
        TextInput textInput("Rename", g_iconEdit, fileSrc);
        if (textInput.execute() != -2 && !textInput.getInputText().empty()) {
            FileUtils::renameFile(m_title + (m_title == "/" ? "" : "/") + fileSrc, m_title + (m_title == "/" ? "" : "/") + textInput.getInputText());
            refresh();
        }
    }
    break;
    default:
        break;
    }
    // If the file was auto-selected, auto-unselect it
    if (autoUnselect) {
        m_fileLister.setSelectedAll(false);
        g_hasChanged = true;
    }
}

//------------------------------------------------------------------------------

// Refresh current directory
void MainWindow::refresh(void) {
    INHIBIT(std::cout << "MainWindow::refresh\n";)
    // List the current path
    if (!m_fileLister.list(m_title)) {
        // Path is wrong, fallback to default
        m_title = START_PATH;
        m_fileLister.list(m_title);
    }
    // Update number of items
    m_nbItems = m_fileLister.getNbElements();
    // Adjust selected line
    if (m_cursor > m_nbItems - 1)
        m_cursor = m_nbItems - 1;
    // Adjust camera
    adjustCamera();
    // Adjust scrollbar
    adjustScrollbar();
    // New render
    g_hasChanged = true;
}

//------------------------------------------------------------------------------

// Adjust camera
void MainWindow::adjustCamera(void) {
    if (m_nbItems <= m_nbVisibleLines)
        m_camera.y = 0;
    else if (m_cursor < m_camera.y)
        m_camera.y = m_cursor;
    else if (m_cursor > m_camera.y + m_nbVisibleLines - 1)
        m_camera.y = m_cursor - m_nbVisibleLines + 1;
}