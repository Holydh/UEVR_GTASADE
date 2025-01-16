// Sanny Builder Library v0.239
/// <reference no-default-lib="true"/>
/// <reference lib="es2020" />
/** Integer value */
type int = (number & { _int: never }) | number;
/** Floating-point value */
type float = (number & { _float: never }) | number;
/** Vector3 type */
type Vector3 = { x: float; y: float; z: float };

/** Pauses the script execution for the specified amount of time in milliseconds */
declare function wait(delay: int): void;
/** Returns a Promise that resolves after given time in milliseconds */
declare function asyncWait(delay: int): Promise<void>;
/** Displays a black text box with custom text. Not available on an `unknown` host */
declare function showTextBox(text: string): void;
/** Prints serialized values to the cleo_redux.log */
declare function log(...values: Array<any>): void;
/** Executes the command by name with the given arguments */
declare function native<T>(name: string, ...args: any[]): T;
/** Terminates the script and optionally writes a reason to the log file */
declare function exit(reason?: string): void;
/** Creates a new event listener https://re.cleo.li/docs/en/events.html */
declare function addEventListener<T>(event: string, callback: (event: CleoEvent<T>) => void): () => void;
/** Dispatches an event https://re.cleo.li/docs/en/events.html */
declare function dispatchEvent<T>(event: string, data?: T): void;
// Sets a timer which executes a function once the timer expires
declare function setTimeout(callback: () => void, delay?: int, ...args: any[]): number;
// Repeatedly calls a function with a fixed time delay between each call
declare function setInterval(callback: () => void, delay?: int, ...args: any[]): number;
// Cancels a timeout previously established by calling setTimeout()
declare function clearTimeout(id: number): void;
// Cancels a timed, repeating action which was previously established by a call to setInterval()
declare function clearInterval(id: number): void;

/** Current host name */
declare const HOST:
  | "re3"
  | "reVC"
  | "gta3"
  | "vc"
  | "sa"
  | "gta3_unreal"
  | "vc_unreal"
  | "sa_unreal"
  | "gta_iv"
  | "bully"
  | "unknown";
/** Is player on a mission flag. Not available on an `unknown` host */
declare var ONMISSION: boolean;
/** Self-incrementing timer #1 */
declare var TIMERA: int;
/** Self-incrementing timer #2 */
declare var TIMERB: int;
/** Current file's directory */
declare const __dirname: string;
/** Current file's path */
declare const __filename: string;

declare interface Version {
  readonly major: string | undefined;
  readonly minor: string | undefined;
  readonly patch: string | undefined;
  readonly pre: string | undefined;
  readonly build: string | undefined;
  toString(): string;
}
declare interface CleoEvent<T = object> {
  name: string;
  data: T | undefined;
}
interface CLEO {
  /** CLEO Redux version */
  readonly version: Version;
  /** Version of API definitions from Sanny Builder Library */
  readonly apiVersion: Version;
  /** Version of the host's exe file. Not available on some hosts */
  readonly hostVersion?: Version;
  debug: {
    /** Enables or disables tracing of executed commands in cleo_redux.log */
    trace(flag: boolean): void;
  };
  /** Spawns a new instance of a script at path and optionally sets initial values for the variables */
  runScript(path: string, args?: object): void;
}

declare const CLEO: CLEO;
interface FxtStore {
  /**
   * Inserts new text content in the script's fxt storage overwriting the previous content and shadowing static fxt with the same key
   * @param key GXT key that can be used in text commands (7 characters max)
   * @param value text content
   * @param [isGlobal] if true, the text affects global FXT storage
   */
  insert(key: string, value: string, isGlobal?: boolean): void;
  /**
   * Removes the text content associated with the key in the local fxt storage
   * @param key GXT key
   * @param [isGlobal] if true, the text affects global FXT storage
   */
  delete(key: string, isGlobal?: boolean): void;
}

declare const FxtStore: FxtStore;
/** Loading DLL files and finding exported functions
 * 
 * https://library.sannybuilder.com/#/unknown_x64/classes/DynamicLibrary */
declare class DynamicLibrary {
    constructor(handle: number);
    /** Loads the specified module (usually a dynamic-link library (DLL)) into the address space of the game
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=LOAD_DYNAMIC_LIBRARY */
    static Load(libraryFileName: string): DynamicLibrary | undefined;
    /** Frees the loaded dynamic-link library (DLL) module and unloads it from the address space of the game
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=FREE_DYNAMIC_LIBRARY */
    free(): void;
    /** Retrieves the address of an exported function or variable from the specified dynamic-link library (DLL)
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=GET_DYNAMIC_LIBRARY_PROCEDURE */
    getProcedure(procName: string): int | undefined;
}
/** 
 * 
 * https://library.sannybuilder.com/#/unknown_x64/classes/File */
declare class File {
    constructor(handle: number);
    /** Opens the file at the specified path. Returns the file identifier (handle), which starts with 0 if the operation was successful, otherwise -1 will be returned
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=OPEN_FILE */
    static Open(filePathName: string): File;
    /** Closes the file and frees the memory
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=CLOSE_FILE */
    close(): void;
    /** Copies data from the source string to the file
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=WRITE_STRING_TO_FILE */
    writeString(source: string): File;
}
/** 
 * 
 * https://library.sannybuilder.com/#/unknown_x64/classes/Game */
interface Game {
    /** Returns game FPS
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=GET_FRAMERATE */
    GetFramerate(): int;
}
declare var Game: Game
/** ImGui integration
 * 
 * https://library.sannybuilder.com/#/unknown_x64/classes/ImGui */
interface ImGui {
    /** Adds a line form point A to B
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_DRAWLIST_ADD_LINE */
    AddLine(drawList: int, p1X: float, p1Y: float, p2X: float, p2Y: float, r: int, g: int, b: int, a: int, thickness: float): void;
    /** Adds text at specified position
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_DRAWLIST_ADD_TEXT */
    AddText(drawList: int, posX: float, posY: float, r: int, g: int, b: int, a: int, text: string): void;
    /** Creates the window
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_BEGIN */
    Begin(windowName: string, state: boolean, noTitleBar: boolean, noResize: boolean, noMove: boolean, autoResize: boolean): boolean;
    /** Creates a child window widget inside the main window
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_BEGIN_CHILD */
    BeginChild(uniqueId: string): void;
    /** Creates a child window widget inside the main window
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_BEGIN_CHILDEX */
    BeginChildEx(uniqueId: string, width: float, height: float, border: boolean, flags: int): void;
    /** Disables ImGui widgets inside this block
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_BEGIN_DISABLED */
    BeginDisabled(disabled: boolean): void;
    /** Creates a unique frame with its own space in memory. End it with 0C32. MUST BE UNIQUE!
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_BEGIN_FRAME */
    BeginFrame(uniqueId: string): void;
    /** Creates the main menu bar
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_BEGIN_MAINMENUBAR */
    BeginMainMenuBar(uniqueId: string): void;
    /** Begins a ImGui menu block
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_BEGIN_MENU */
    BeginMenu(label: string, enabled: boolean): boolean;
    /** Creates a bullet point
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_BULLET */
    Bullet(): void;
    /** Creates the button
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_BUTTON */
    Button(buttonName: string, width: float, height: float): boolean;
    /** Creates the arrow button in the specified direction
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_ARROW_BUTTON */
    ButtonArrow(name: string, imGuiDir: int): boolean;
    /** Creates the button with custom colors
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_COLOR_BUTTON */
    ButtonColored(buttonName: string, red: float, green: float, blue: float, alpha: float, width: float, height: float): boolean;
    /** Creates a ImGui button with specified image
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_IMAGE_BUTTON */
    ButtonImage(name: string, image: int, width: float, height: float): boolean;
    /** Creates the invisible button
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_INVISIBLE_BUTTON */
    ButtonInvisible(buttonName: string, width: float, height: float): boolean;
    /** Returns the width and height of the given text
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_CALC_TEXT_SIZE */
    CalcTextSize(text: string): {
        width: float;
        height: float;
    };
    /** Creates the checkbox
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_CHECKBOX */
    Checkbox(label: string, isChecked: boolean): boolean;
    /** Adds the collapsing header
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_COLLAPSING_HEADER */
    CollapsingHeader(label: string): boolean;
    /** Creates the color picker and sets the default color (0-255)
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_COLOR_PICKER */
    ColorPicker(label: string): {
        red: int;
        green: int;
        blue: int;
        alpha: int;
    };
    /** Divides the window width into N columns. Close this with Columns(1)
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_COLUMNS */
    Columns(count: int): void;
    /** Creates a combo box widget. Pass options separated by commas "item1,item2,item3"
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_COMBO */
    ComboBox(name: string, options: string, selection: int): int;
    /** Creates the dummy widget. Used for spacing
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_DUMMY */
    Dummy(width: float, height: float): void;
    /** Ends the window
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_END */
    End(): void;
    /** Ends the child window widget created with 0C25
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_END_CHILD */
    EndChild(): void;
    /** Closes the ImGui disable block
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_END_DISABLED */
    EndDisabled(): void;
    /** Ends unique ImGui frame
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_END_FRAME */
    EndFrame(): void;
    /** Ends the main menu bar
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_END_MAINMENUBAR */
    EndMainMenuBar(): void;
    /** Ends a ImGui menu block
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_END_MENU */
    EndMenu(): void;
    /** Frees a loaded image data
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_FREE_IMAGE */
    FreeImage(image: int): void;
    /** Returns pointer to ImGui background drawlist
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_GET_BACKGROUND_DRAWLIST */
    GetBackgroundDrawList(): int;
    /** Returns the width & height of the display
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_GET_DISPLAY_SIZE */
    GetDisplaySize(): {
        width: float;
        height: float;
    };
    /** Returns pointer to foreground draw list
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_GET_FOREGROUND_DRAWLIST */
    GetForegroundDrawList(): int;
    /** Returns the ImGui frame height
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_GET_FRAME_HEIGHT */
    GetFrameHeight(): float;
    /** Returns the ImGuiRedux version
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_GET_PLUGIN_VERSION */
    GetPluginVersion(): int;
    /** Returns the width and height scaling factor based on the window size
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_GET_SCALING_SIZE */
    GetScalingSize(uniqueId: string, count: int, spacing: boolean): {
        x: float;
        y: float;
    };
    /** Returns the ImGui version
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_GET_VERSION */
    GetVersion(): int;
    /** Returns the content region width of the window
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_GET_WINDOW_CONTENT_REGION_WIDTH */
    GetWindowContentRegionWidth(uniqueId: string): float;
    /** Returns pointer to ImGui window drawList
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_GET_WINDOW_DRAWLIST */
    GetWindowDrawlist(): int;
    /** Returns the x,y coordinates of the window on the screen
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_GET_WINDOW_POS */
    GetWindowPos(uniqueId: string): {
        x: float;
        y: float;
    };
    /** Returns the width and height of the window
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_GET_WINDOW_SIZE */
    GetWindowSize(uniqueId: string): {
        width: float;
        height: float;
    };
    /** Creates the float input
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_INPUT_FLOAT */
    InputFloat(label: string, initValue: float, min: float, max: float): float;
    /** Creates the int input
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_INPUT_INT */
    InputInt(label: string, initValue: int, min: int, max: int): int;
    /** Creates the text input
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_INPUT_TEXT */
    InputText(label: string): string;
    /** Returns true if the previous widget is in active state
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_IS_ITEM_ACTIVE */
    IsItemActive(uniqueId: string): boolean;
    /** Returns true if the previous widget is clicked
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_IS_ITEM_CLICKED */
    IsItemClicked(uniqueId: string): boolean;
    /** Returns true if the previous widget is focused
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_IS_ITEM_FOCUSED */
    IsItemFocused(uniqueId: string): boolean;
    /** Returns true if the previous widget is hovered with mouse
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_IS_ITEM_HOVERED */
    IsItemHovered(uniqueId: string): boolean;
    /** Loads a image file from disk. Relative to CLEO directory
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_LOAD_IMAGE */
    LoadImage(path: string): int;
    /** Adds the menu item
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_MENU_ITEM */
    MenuItem(text: string, selected: boolean, enabled: boolean): boolean;
    /** Creates a new line for the next widget
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_NEWLINE */
    NewLine(): void;
    /** Puts the next widgets on the next column. Used alongside 0C16
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_NEXT_COLUMN */
    NextColumn(): void;
    /** Removes the pushed item width (0C27) from the stack
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_POP_ITEM_WIDTH */
    PopItemWidth(): void;
    /** Removes the recent ImGuiCol from the stack
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_POP_STYLE_COLOR */
    PopStyleColor(count: int): void;
    /** Removes the recent imGuiStyleVar from the stack
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_POP_STYLE_VAR */
    PopStyleVar(count: int): void;
    /** Sets the item width for the next widgets
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_PUSH_ITEM_WIDTH */
    PushItemWidth(width: float): void;
    /** Pushes a ImGuiCol value to the stack. Use PopStyleColor to undo the effect
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_PUSH_STYLE_COLOR */
    PushStyleColor(imGuiCol: int, r: int, g: int, b: int, a: int): void;
    /** Pushes a ImGuiStyleVar value to the stack. Use PopStyleVar to undo the effect
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_PUSH_STYLE_VAR */
    PushStyleVar(imGuiStyleVar: int, val: float): void;
    /** Pushes a ImGuiStyleVar value to the stack. Use PopStyleVar to undo the effect
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_PUSH_STYLE_VAR2 */
    PushStyleVar2(imGuiStyleVar: int, x: float, y: float): void;
    /** Creates the radio button
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_RADIO_BUTTON */
    RadioButton(label: string, selectedBtn: int, btnNo: int): int;
    /** Appends the next widget to the same line as the previous widget
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SAMELINE */
    SameLine(): void;
    /** Adds the selectable widget
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SELECTABLE */
    Selectable(text: string, selected: boolean): boolean;
    /** Adds a horizontal separator line
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SEPARATOR */
    Separator(): void;
    /** Sets the width of the column
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_COLUMN_WIDTH */
    SetColumnWidth(index: int, width: float): void;
    /** Toggles the cursor
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_CURSOR_VISIBLE */
    SetCursorVisible(show: boolean): void;
    /** Sets image background color
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_IMAGE_BG_COLOR */
    SetImageBgColor(r: float, g: float, b: float, a: float): void;
    /** Sets image tint color
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_IMAGE_TINT_COLOR */
    SetImageTintColor(r: float, g: float, b: float, a: float): void;
    /** Sets the value of input float & slider float widget
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_ITEM_FLOAT */
    SetItemValueFloat(id: string, val: float): void;
    /** Sets the value of input int & slider int widget
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_ITEM_INT */
    SetItemValueInt(id: string, val: int): void;
    /** Sets value of input text widget
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_ITEM_TEXT */
    SetItemValueText(id: string, val: string): void;
    /** Displays a text message on top left corner of the screen. Useful for games without `showTextBox(...)` support
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_MESSAGE */
    SetMessage(text: string): void;
    /** Sets the current window position. Applies to the next window ( aka Begin() )
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_NEXT_WINDOW_POS */
    SetNextWindowPos(x: float, y: float, imGuiCond: int): void;
    /** Sets the current window size. Applies to the next window ( aka Begin() )
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_NEXT_WINDOW_SIZE */
    SetNextWindowSize(width: float, height: float, imGuiCond: int): void;
    /** Sets the background transparency of next window (0.0f-1.0f)
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_NEXT_WINDOW_TRANSPARENCY */
    SetNextWindowTransparency(alpha: float): void;
    /** Creates the popup window with the given text
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_TOOLTIP */
    SetTooltip(text: string): void;
    /** Sets the current window position. Must be called inside Begin()...End()
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_WINDOW_POS */
    SetWindowPos(x: float, y: float, imGuiCond: int): void;
    /** Sets the current window size. Must be called inside Begin()...End()
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SET_WINDOW_SIZE */
    SetWindowSize(width: float, height: float, imGuiCond: int): void;
    /** Creates the float slider input
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SLIDER_FLOAT */
    SliderFloat(label: string, initValue: float, min: float, max: float): float;
    /** Creates the int slider input
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SLIDER_INT */
    SliderInt(label: string, initValue: int, min: int, max: int): int;
    /** Adds some spacing after the previous widget
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_SPACING */
    Spacing(): void;
    /** Pass tab names separated by comma. Returns the index of the visible tab
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_TABS */
    Tabs(name: string, tabNames: string): int;
    /** Creates the text line
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_TEXT */
    Text(text: string): void;
    /** Displays a center aligned ImGui text widget
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_TEXT_CENTERED */
    TextCentered(text: string): void;
    /** Creates the text line of the given RGBA color (0.0f-1.0f)
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_TEXT_COLORED */
    TextColored(text: string, red: float, green: float, blue: float, alpha: float): void;
    /** Creates the text line with the disabled color ( Grayish by default )
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_TEXT_DISABLED */
    TextDisabled(text: string): void;
    /** Creates the text line with a bullet point
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_BULLET_TEXT */
    TextWithBullet(text: string): void;
    /** Creates the text line that wraps to a newline if the text goes beyond the window width
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IMGUI_TEXT_WRAPPED */
    TextWrapped(text: string): void;
}
declare var ImGui: ImGui
/** Reading and writing .ini files
 * 
 * https://library.sannybuilder.com/#/unknown_x64/classes/IniFile */
interface IniFile {
    /** Reads a floating-point value from the ini file
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=READ_FLOAT_FROM_INI_FILE */
    ReadFloat(path: string, section: string, key: string): float | undefined;
    /** Reads an integer value from the ini file
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=READ_INT_FROM_INI_FILE */
    ReadInt(path: string, section: string, key: string): int | undefined;
    /** Reads a string value from the ini file
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=READ_STRING_FROM_INI_FILE */
    ReadString(path: string, section: string, key: string): string | undefined;
    /** Writes the floating-point value to the ini file
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=WRITE_FLOAT_TO_INI_FILE */
    WriteFloat(value: float, path: string, section: string, key: string): boolean;
    /** Writes the integer value to the ini file
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=WRITE_INT_TO_INI_FILE */
    WriteInt(value: int, path: string, section: string, key: string): boolean;
    /** Writes the string value to the ini file
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=WRITE_STRING_TO_INI_FILE */
    WriteString(value: string, path: string, section: string, key: string): boolean;
}
declare var IniFile: IniFile
/** 
 * 
 * https://library.sannybuilder.com/#/unknown_x64/classes/Memory */
interface Memory {
    /** Reads a floating-point value (IEEE 754) from the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ReadFloat(address: int, vp: boolean, ib: boolean): float;
    /** Writes a floating-point value (IEEE 754) to the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    WriteFloat(address: int, value: float, vp: boolean, ib: boolean): void;
    /** Reads a 8-bit signed integer value from the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ReadI8(address: int, vp: boolean, ib: boolean): int;
    /** Reads a 16-bit signed integer value from the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ReadI16(address: int, vp: boolean, ib: boolean): int;
    /** Reads a 32-bit signed integer value from the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ReadI32(address: int, vp: boolean, ib: boolean): int;
    /** Reads a 64-bit signed integer value from the memory
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ReadI64(address: int, vp: boolean, ib: boolean): int;
    /** Reads a 8-bit unsigned integer value from the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ReadU8(address: int, vp: boolean, ib: boolean): int;
    /** Reads a 16-bit unsigned integer value from the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ReadU16(address: int, vp: boolean, ib: boolean): int;
    /** Reads a 32-bit unsigned integer value from the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ReadU32(address: int, vp: boolean, ib: boolean): int;
    /** Reads a 64-bit unsigned integer value from the memory
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ReadU64(address: int, vp: boolean, ib: boolean): int;
    /** Reads a null-terminated UTF-8 encoded string from the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ReadUtf8(address: int, ib: boolean): string;
    /** Reads a null-terminated UTF-16 encoded string from the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ReadUtf16(address: int, ib: boolean): string;
    /** Writes a 8-bit signed integer value to the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    WriteI8(address: int, value: int, vp: boolean, ib: boolean): void;
    /** Writes a 16-bit signed integer value to the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    WriteI16(address: int, value: int, vp: boolean, ib: boolean): void;
    /** Writes a 32-bit signed integer value to the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    WriteI32(address: int, value: int, vp: boolean, ib: boolean): void;
    /** Writes a 64-bit signed integer value to the memory
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    WriteI64(address: int, value: int, vp: boolean, ib: boolean): void;
    /** Writes a 8-bit unsigned integer value to the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    WriteU8(address: int, value: int, vp: boolean, ib: boolean): void;
    /** Writes a 16-bit unsigned integer value to the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    WriteU16(address: int, value: int, vp: boolean, ib: boolean): void;
    /** Writes a 32-bit unsigned integer value to the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    WriteU32(address: int, value: int, vp: boolean, ib: boolean): void;
    /** Writes a 64-bit unsigned integer value to the memory
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    WriteU64(address: int, value: int, vp: boolean, ib: boolean): void;
    /** Writes a sequence of UTF-8 encoded characters to the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    WriteUtf8(address: int, value: string, vp: boolean, ib: boolean): void;
    /** Writes a sequence of UTF-16 encoded characters to the memory 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    WriteUtf16(address: int, value: string, vp: boolean, ib: boolean): void;
    /** Reads 1, 2, 4, or 8 bytes from the memory
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    Read(address: int, size: int, vp: boolean, ib: boolean): int;
    /** Writes 1, 2, 4, or 8 bytes to the memory. Can also be used as memset.
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    Write(address: int, size: int, value: int, vp: boolean, ib: boolean): void;

    /** Cast 32-bit signed integer value to floating-point value (IEEE 754) 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ToFloat(value: int): float;
    /** Cast floating-point value (IEEE 754) to 32-bit signed integer value 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    FromFloat(value: float): int;
    /** Cast 8-bit signed integer value to unsigned integer value 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ToU8(value: int): int;
    /** Cast 16-bit signed integer value to unsigned integer value 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ToU16(value: int): int;
    /** Cast 32-bit signed integer value to unsigned integer value 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ToU32(value: int): int;
    /** Cast 8-bit unsigned integer value to signed integer value 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ToI8(value: int): int;
    /** Cast 16-bit unsigned integer value to signed integer value 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ToI16(value: int): int;
    /** Cast 32-bit unsigned integer value to signed integer value 
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    ToI32(value: int): int;
    /** Returns address of a function or variable, or 0 otherwise
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    Translate(symbol: string): int;

    /** Convenience methods for invoking different types of functions */
    Fn: {
        /** Creates a new function to be called using "x64" calling convention
         * https://en.wikipedia.org/wiki/X86_calling_conventions#Microsoft_x64_calling_convention
         * @param address either absolute address or the relative offset of the function
         * @param ib should treat the address as the offset relative to the IMAGE BASE 
         * @returns a function accepting arguments and returning a 32-bit signed integer value as a result */
        X64(address: int, ib: boolean): (...funcParams: int[]) => int;
        /** Creates a new function to be called using "x64" calling convention
         * https://en.wikipedia.org/wiki/X86_calling_conventions#Microsoft_x64_calling_convention
         * @param address either absolute address or the relative offset of the function
         * @param ib should treat the address as the offset relative to the IMAGE BASE 
         * @returns a function accepting arguments and returning a floating-point value as a result */
        X64Float(address: int, ib: boolean): (...funcParams: int[]) => float;
        /** Creates a new function to be called using "x64" calling convention
         * https://en.wikipedia.org/wiki/X86_calling_conventions#Microsoft_x64_calling_convention
         * @param address either absolute address or the relative offset of the function
         * @param ib should treat the address as the offset relative to the IMAGE BASE 
         * @returns a function accepting arguments and returning a 8-bit signed integer value as a result */
        X64I8(address: int, ib: boolean): (...funcParams: int[]) => int;
        /** Creates a new function to be called using "x64" calling convention
         * https://en.wikipedia.org/wiki/X86_calling_conventions#Microsoft_x64_calling_convention
         * @param address either absolute address or the relative offset of the function
         * @param ib should treat the address as the offset relative to the IMAGE BASE 
         * @returns a function accepting arguments and returning a 16-bit signed integer value as a result */
        X64I16(address: int, ib: boolean): (...funcParams: int[]) => int;
        /** Creates a new function to be called using "x64" calling convention
         * https://en.wikipedia.org/wiki/X86_calling_conventions#Microsoft_x64_calling_convention
         * @param address either absolute address or the relative offset of the function
         * @param ib should treat the address as the offset relative to the IMAGE BASE 
         * @returns a function accepting arguments and returning a 32-bit signed integer value as a result */
        X64I32(address: int, ib: boolean): (...funcParams: int[]) => int;
        /** Creates a new function to be called using "x64" calling convention
         * https://en.wikipedia.org/wiki/X86_calling_conventions#Microsoft_x64_calling_convention
         * @param address either absolute address or the relative offset of the function
         * @param ib should treat the address as the offset relative to the IMAGE BASE 
         * @returns a function accepting arguments and returning a 8-bit unsigned integer value as a result */
        X64U8(address: int, ib: boolean): (...funcParams: int[]) => int;
        /** Creates a new function to be called using "x64" calling convention
         * https://en.wikipedia.org/wiki/X86_calling_conventions#Microsoft_x64_calling_convention
         * @param address either absolute address or the relative offset of the function
         * @param ib should treat the address as the offset relative to the IMAGE BASE 
         * @returns a function accepting arguments and returning a 16-bit unsigned integer value as a result */
        X64U16(address: int, ib: boolean): (...funcParams: int[]) => int;
        /** Creates a new function to be called using "x64" calling convention
         * https://en.wikipedia.org/wiki/X86_calling_conventions#Microsoft_x64_calling_convention
         * @param address either absolute address or the relative offset of the function
         * @param ib should treat the address as the offset relative to the IMAGE BASE 
         * @returns a function accepting arguments and returning a 32-bit unsigned integer value as a result */
        X64U32(address: int, ib: boolean): (...funcParams: int[]) => int;
    }

    /** Calls a function
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    CallFunction(address: int, ib: boolean, numParams: int, ...funcParams: int[]): void;

    /** Calls a function that returns an integer value
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    CallFunctionReturn(address: int, ib: boolean, numParams: int, ...funcParams: int[]): int;

    /** Calls a function that returns a floating-point value
    *
    * https://re.cleo.li/docs/en/using-memory-64.html */
    CallFunctionReturnFloat(address: int, ib: boolean, numParams: int, ...funcParams: int[]): float;
    /** Allocates a chunk of memory of the given size near to the memory page of the main exe module
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=ALLOC_NEAR */
    AllocNear(size: int): int;
    /** Returns an address of a memory chunk with the given index in a list of matches for the pattern
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=FIND_PATTERN */
    FindPattern(pattern: string, index: int): int | undefined;
    /** Returns the address of the main exe module
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=GET_IMAGE_BASE */
    GetImageBase(): int;
    /** Reads a 32-bit value referenced by a relative offset at the address
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=READ_RELATIVE_OFFSET */
    ReadRelativeOffset(address: int): int;
    /** Replaces an offset at the address with the offset to the near address (use ALLOC_NEAR)
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=WRITE_RELATIVE_OFFSET */
    WriteRelativeOffset(address: int, nearAddress: int): void;
}
declare var Memory: Memory
/** 
 * 
 * https://library.sannybuilder.com/#/unknown_x64/classes/Mouse */
interface Mouse {
    /** Returns the position of the mouse cursor
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=GET_CURSOR_POS */
    GetCursorPos(): {
        x: int;
        y: int;
    } | undefined;
    /** Sets the position of the mouse cursor
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=SET_CURSOR_POS */
    SetCursorPos(x: int, y: int): boolean;
}
declare var Mouse: Mouse
/** 
 * 
 * https://library.sannybuilder.com/#/unknown_x64/classes/Pad */
interface Pad {
    /** Returns the code of the last pressed button
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=GET_LAST_KEY */
    GetLastKey(): int;
    /** Holds down a keyboard or mouse button until it gets released with RELEASE_KEY
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=HOLD_KEY */
    HoldKey(keyCode: int): void;
    /** Returns true if a keyboard or mouse button has just been pressed
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IS_KEY_DOWN */
    IsKeyDown(keyCode: int): boolean;
    /** Returns true if a keyboard or mouse button is being pressed
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IS_KEY_PRESSED */
    IsKeyPressed(keyCode: int): boolean;
    /** Returns true if a keyboard or mouse button has just been released
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=IS_KEY_UP */
    IsKeyUp(keyCode: int): boolean;
    /** Releases a keyboard or mouse button after HOLD_KEY
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=RELEASE_KEY */
    ReleaseKey(keyCode: int): void;
    /** Returns true if the specified sequence of alphanumeric characters has just been typed on the keyboard
    *
    * https://library.sannybuilder.com/#/unknown_x64?q=TEST_CHEAT */
    TestCheat(input: string): boolean;
}
declare var Pad: Pad
