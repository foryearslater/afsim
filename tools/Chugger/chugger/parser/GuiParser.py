# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

from tkinter import *
from tkinter import messagebox, BooleanVar
from tkinter import filedialog

import threading
import shlex
import time
from collections import deque

from chugger.parser.BaseParser import BaseParser
from chugger import StreamInterface
from chugger.parser.ParseSpace import ParseSpace


class GuiParser(BaseParser):
    """
    GuiParser is the GUI frontend version of the CmdParser.
    This class contains all helper functions and asset definitions
    required to launch a GUI interface.
    """
    def __init__(self, stream: StreamInterface):
        """
        :param stream:
            StreamInterface object that allows for interprocess communication
        """
        # Variables
        print("This is the raw terminal window for all error information regarding Chugger.\n"
              "If you see any errors within this window please report them.")
        self.stream = stream
        self.top_window = Tk()
        self.args = ParseSpace()
        self.support_windows = BooleanVar(False)  # 1 == True ; 0 == False
        self.support_linux = BooleanVar(False)  # 1 == True ; 0 == False
        self.is_server = IntVar(0)

        self.top_window.title("Chugger GUI Frontend")
        self.top_window.resizable(False, False)
        # tkinter assets
        self.lbframe_inputs = LabelFrame(self.top_window, text="Main file inputs")
        self.frame_startup = Frame(self.lbframe_inputs)
        self.label_startup = Label(self.frame_startup, text="AFSIM Startup File:", width=20)
        self.entry_startup = Entry(self.frame_startup, width=50)
        self.button_startup = Button(self.frame_startup, text="browse",
                                     command=lambda: self.browse_file(self.entry_startup,
                                                                      "AFSIM Startup File",
                                                                      [("Text file", "*.txt")]))
        self.frame_matrixinput = Frame(self.lbframe_inputs)
        self.label_matrixinput = Label(self.frame_matrixinput, text="Matrix File:", width=20)
        self.entry_matrixinput = Entry(self.frame_matrixinput, width=50)
        self.button_matrixinput = Button(self.frame_matrixinput, text="browse",
                                         command=lambda: self.browse_file(self.entry_matrixinput, "Matrix File",
                                                                          [("CSV file", "*.csv")]))
        self.frame_output = Frame(self.lbframe_inputs)
        self.label_output = Label(self.frame_output, text="Output Folder:", width=20)
        self.entry_output = Entry(self.frame_output, width=50)
        self.button_output = Button(self.frame_output, text="browse",
                                    command=lambda: self.browse_folder(self.entry_output, "Output Folder"))
        self.lbframe_optional = LabelFrame(self.top_window, text="Optional modifiers")
        self.frame_runs = Frame(self.lbframe_optional)
        self.label_runs = Label(self.frame_runs, text="Number of Runs:", width=20)
        self.entry_runs = Entry(self.frame_runs, width=50)
        self.frame_dpr = Frame(self.lbframe_optional)
        self.label_dpr = Label(self.frame_dpr, text="Data-Point Range:", width=20)
        self.entry_dp_min = Entry(self.frame_dpr, width=25)
        self.entry_dp_max = Entry(self.frame_dpr, width=25)
        self.frame_variable = Frame(self.lbframe_optional)
        self.label_variable = Label(self.frame_variable, text="Variable File:", width=20)
        self.entry_variable = Entry(self.frame_variable, width=50)
        self.button_variable = Button(self.frame_variable, text="Browse",
                                      command=lambda: self.browse_file(self.entry_variable, "Variable File",
                                                                       [("Text file", "*.txt")]))
        self.frame_run_variable = Frame(self.lbframe_optional)
        self.label_run_variable = Label(self.frame_run_variable, text="Run Variable:", width=20)
        self.entry_run_variable = Entry(self.frame_run_variable, width=50)
        self.lbframe_manager = LabelFrame(self.top_window, text="Manager type")
        self.radio_server = Radiobutton(self.lbframe_manager, text="Server Manager",
                                        variable=self.is_server, value=1,
                                        command=lambda: self.display_server_options())
        self.radio_local = Radiobutton(self.lbframe_manager, text="Local Manager",
                                       variable=self.is_server, value=0,
                                       command=lambda: self.display_local_options())
        self.lbframe_local = LabelFrame(self.top_window, text="Local Options")
        self.frame_lthreads = Frame(self.lbframe_local)
        self.label_lthreads = Label(self.frame_lthreads, text="Local Threads:")
        self.entry_lthreads = Entry(self.frame_lthreads, width=25)
        self.frame_lexe = Frame(self.lbframe_local)
        self.label_lexe = Label(self.frame_lexe, text="Local Executable:")
        self.entry_lexe = Entry(self.frame_lexe)
        self.button_lexe = Button(self.frame_lexe, text="Browse",
                                  command=lambda: self.browse_file(self.entry_lexe,
                                                                   "Local executable", [("EXE file", "*.exe")]))
        self.lbframe_server = LabelFrame(self.top_window, text="Server Options")
        self.frame_ip = Frame(self.lbframe_server)
        self.label_ip = Label(self.frame_ip, text="IP Address:")
        self.entry_ip = Entry(self.frame_ip, width=50)
        self.frame_port = Frame(self.lbframe_server)
        self.label_port = Label(self.frame_port, text="Port Number:")
        self.entry_port = Entry(self.frame_port, width=50)
        self.frame_scenario = Frame(self.lbframe_server)
        self.label_scenario = Label(self.frame_scenario, text="Scenario Folder:")
        self.entry_scenario = Entry(self.frame_scenario, width=50)
        self.button_scenario = Button(self.frame_scenario, text="Browse",
                                      command=lambda: self.browse_folder(self.entry_scenario,
                                                                         "Scenario Folder"))
        self.lbframe_support = LabelFrame(self.top_window, text="Platform support")
        self.check_winsupport = Checkbutton(self.lbframe_support, text="Support Windows Clients",
                                            onvalue=True, offvalue=False, variable=self.support_windows,
                                            command=lambda: self.display_platform_options())
        self.check_linsupport = Checkbutton(self.lbframe_support, text="Support Linux Clients",
                                            onvalue=True, offvalue=False, variable=self.support_linux,
                                            command=lambda: self.display_platform_options())
        self.lbframe_linux = LabelFrame(self.top_window, text="Linux support")
        self.frame_linbin = Frame(self.lbframe_linux)
        self.label_linbin = Label(self.frame_linbin, text="Linux AFSIM Bin:")
        self.entry_linbin = Entry(self.frame_linbin, width=50)
        self.button_linbin = Button(self.frame_linbin, text="Browse",
                                    command=lambda: self.browse_folder(self.entry_linbin, "Linux AFSIM Bin Folder"))
        self.frame_linexe = Frame(self.lbframe_linux)
        self.label_linexe = Label(self.frame_linexe, text="Linux AFSIM Mission:")
        self.entry_linexe = Entry(self.frame_linexe, width=50)
        self.button_linexe = Button(self.frame_linexe, text="Browse",
                                    command=lambda: self.browse_file(self.entry_linexe,
                                                                     "Linux AFSIM Mission executable",
                                                                     [("*nix file", "*")]))
        self.lbframe_windows = LabelFrame(self.top_window, text="Windows support")
        self.frame_winbin = Frame(self.lbframe_windows)
        self.label_winbin = Label(self.frame_winbin, text="Windows AFSIM Bin:")
        self.entry_winbin = Entry(self.frame_winbin, width=50)
        self.button_winbin = Button(self.frame_winbin, text="Browse",
                                    command=lambda: self.browse_folder(self.entry_winbin,
                                                                       "Windows AFSIM Bin Folder"))
        self.frame_winexe = Frame(self.lbframe_windows)
        self.label_winexe = Label(self.frame_winexe, text="Windows AFSIM Mission:")
        self.entry_winexe = Entry(self.frame_winexe, width=50)
        self.button_winexe = Button(self.frame_winexe, text="Browse",
                                    command=lambda: self.browse_file(self.entry_winexe,
                                                                     "Windows AFSIM Mission executable",
                                                                     [("EXE file", "*.exe")]))

        self.terminal_window = Toplevel()
        self.update_event = threading.Event()
        self.text_box = Text(self.terminal_window, width=100, height=50)

        self.frame_app_controller = Frame(self.top_window)
        self.button_close = Button(self.frame_app_controller, width=15, text="Exit",
                                   command=lambda: self.close_manager())
        self.button_start = Button(self.frame_app_controller, width=15, text="Start",
                                   command=lambda: self.prelaunch_mgr())
        self.button_term = Button(self.frame_app_controller, width=15, text="Terminal",
                                  command=lambda: self.open_terminal())
        self.frame_proc_controller = Frame(self.top_window)
        self.button_load = Button(self.frame_proc_controller, width=15, text="Load Script",
                                  command=lambda: self.load_script())
        self.button_generate = Button(self.frame_proc_controller, width=15, text="Generate Script",
                                      command=lambda: self.prep_create_script())
        self.button_kill = Button(self.frame_proc_controller, width=15, text="Kill Processes",
                                  command=lambda: self.kill_processes())

        self.frame_startup.grid(column=0, row=0)
        self.label_startup.pack(side=LEFT)
        self.entry_startup.pack(side=LEFT)
        self.button_startup.pack(side=LEFT)
        self.frame_matrixinput.grid(column=0, row=1)
        self.label_matrixinput.pack(side=LEFT)
        self.entry_matrixinput.pack(side=LEFT)
        self.button_matrixinput.pack(side=LEFT)
        self.frame_output.grid(column=0, row=2)
        self.label_output.pack(side=LEFT)
        self.entry_output.pack(side=LEFT)
        self.button_output.pack(side=LEFT)
        self.frame_runs.grid(column=0, row=0)
        self.label_runs.pack(side=LEFT)
        self.entry_runs.pack(side=LEFT)
        self.frame_dpr.grid(column=0, row=1)
        self.label_dpr.pack(side=LEFT)
        self.entry_dp_min.pack(side=LEFT)
        self.entry_dp_max.pack(side=LEFT)
        self.frame_variable.grid(column=0, row=2)
        self.label_variable.pack(side=LEFT)
        self.entry_variable.pack(side=LEFT)
        self.button_variable.pack(side=LEFT)
        self.label_run_variable.pack(side=LEFT)
        self.entry_run_variable.pack(side=LEFT)
        self.frame_run_variable.grid(column=0, row=3)
        self.frame_lthreads.grid(column=0, row=0)
        self.label_lthreads.pack(side=LEFT)
        self.entry_lthreads.pack(side=LEFT)
        self.frame_lexe.grid(column=1, row=0)
        self.label_lexe.pack(side=LEFT)
        self.entry_lexe.pack(side=LEFT)
        self.button_lexe.pack(side=LEFT)
        self.frame_ip.grid(column=0, row=0)
        self.label_ip.pack(side=LEFT)
        self.entry_ip.pack(side=LEFT)
        self.frame_port.grid(column=0, row=1)
        self.label_port.pack(side=LEFT)
        self.entry_port.pack(side=LEFT)
        self.frame_scenario.grid(column=0, row=2)
        self.label_scenario.pack(side=LEFT)
        self.entry_scenario.pack(side=LEFT)
        self.button_scenario.pack(side=LEFT)
        self.frame_linbin.grid(column=0, row=0)
        self.label_linbin.pack(side=LEFT)
        self.entry_linbin.pack(side=LEFT)
        self.button_linbin.pack(side=LEFT)
        self.check_winsupport.grid(column=1, row=0)
        self.check_linsupport.grid(column=0, row=0)
        self.frame_linexe.grid(column=0, row=1)
        self.label_linexe.pack(side=LEFT)
        self.entry_linexe.pack(side=LEFT)
        self.button_linexe.pack(side=LEFT)
        self.frame_winbin.grid(column=0, row=0)
        self.label_winbin.pack(side=LEFT)
        self.entry_winbin.pack(side=LEFT)
        self.button_winbin.pack(side=LEFT)
        self.frame_winexe.grid(column=0, row=1)
        self.label_winexe.pack(side=LEFT)
        self.entry_winexe.pack(side=LEFT)
        self.button_winexe.pack(side=LEFT)
        self.radio_local.grid(column=0, row=0)
        self.radio_server.grid(column=1, row=0)
        self.lbframe_inputs.grid(column=0, row=0, padx=20, pady=5)
        self.lbframe_optional.grid(column=1, row=0, padx=20, pady=5)
        self.lbframe_manager.grid(column=0, row=1, padx=20, pady=5, columnspan=2)
        self.button_start.grid(column=0, row=0)
        self.button_close.grid(column=1, row=0)
        self.button_term.grid(column=2, row=0)
        self.button_generate.grid(column=1, row=0)
        self.button_load.grid(column=0, row=0)
        self.button_kill.grid(column=2, row=0)
        self.frame_app_controller.grid(column=0, row=6, columnspan=2)

        self.start_manager_lock = False
        self.top_window.protocol("WM_DELETE_WINDOW", self.close_manager)
        self.terminal_window.protocol("WM_DELETE_WINDOW", self.close_terminal)
        self.thread_list = list()

        self.text_box.pack(expand=True, fill='both')
        self.terminal_thread = threading.Thread(name="Terminal_output", target=self.update_terminal)
        self.terminal_thread.start()
        self.thread_list.append(self.terminal_thread)

        self.display_local_options()
        self.top_window.mainloop()

    def browse_file(self, entry_field, msg: str, file_type):
        """
        delete the previous entry filed data and replace
        it with a selected filepath
        :param entry_field:
            entry widget to be modified
        :param msg:
            Title message of the prompt
        :param file_type:
            File types to limit for
        :return:
        """
        entry_field.delete(0, END)
        entry_field.insert(0, filedialog.askopenfilename(title=msg, filetypes=file_type))

    def browse_folder(self, entry_field, msg: str):
        """
        delete the previous entry and replace it with a selected
        folder
        :param entry_field:
            entry field to be modified
        :param msg:
            title message of the prompt
        :return:
        """
        entry_field.delete(0, END)
        entry_field.insert(0, filedialog.askdirectory(title=msg))

    def insert_entry_text(self, entry_field, msg: str):
        """
        Inserts text into a given entry
        :param entry_field:
            entry to be modified
        :param msg:
            title message of the prompt
        :return:
        """
        entry_field.delete(0, END)
        entry_field.insert(0, msg)

    def display_server_options(self):
        for widget in self.top_window.grid_slaves():
            if widget.grid_info() == self.lbframe_local.grid_info() or \
                    widget.grid_info() == self.frame_proc_controller.grid_info():
                widget.grid_forget()
        if self.support_linux.get() == 1 or self.support_windows == 1:
            self.display_platform_options()
        self.lbframe_server.grid(column=0, row=2, padx=20, pady=5, columnspan=2)
        self.lbframe_support.grid(column=0, row=3, padx=20, pady=5, columnspan=2)

    def display_platform_options(self):
        if self.support_linux.get() == 0:
            for widget in self.top_window.grid_slaves():
                if widget.grid_info() == self.lbframe_linux.grid_info():
                    widget.grid_forget()
        else:
            self.lbframe_linux.grid(row=4, column=0, padx=20, pady=5)
        if self.support_windows.get() == 0:
            for widget in self.top_window.grid_slaves():
                if widget.grid_info() == self.lbframe_windows.grid_info():
                    widget.grid_forget()
        else:
            self.lbframe_windows.grid(row=4, column=1, padx=20, pady=5)
        if self.support_linux.get() == 1 or self.support_windows.get() == 1:
            self.frame_proc_controller.grid(row=5, column=0, padx=20, pady=5, columnspan=2)
        elif self.support_linux.get() == 0 and self.support_windows.get() == 0:
            for widget in self.top_window.grid_slaves():
                if widget.grid_info() == self.lbframe_windows.grid_info() or \
                        widget.grid_info() == self.lbframe_linux.grid_info() or \
                        widget.grid_info() == self.frame_proc_controller.grid_info():
                    widget.grid_forget()

    def display_local_options(self):
        for widget in self.top_window.grid_slaves():
            if widget.grid_info() == self.lbframe_server.grid_info() or \
                    widget.grid_info() == self.lbframe_support.grid_info() or \
                    widget.grid_info() == self.lbframe_windows.grid_info() or \
                    widget.grid_info() == self.lbframe_linux.grid_info():
                widget.grid_forget()
        self.lbframe_local.grid(row=2, column=0, padx=20, pady=5, columnspan=2)
        self.frame_proc_controller.grid(row=5, column=0, padx=20, pady=5, columnspan=2)

    def display_msg(self, title: str, msg: str):
        """
        displays messagebox with information
        :param title:
            title of the given prompt
        :param msg:
            message to be displayed
        :return:
        """
        messagebox.showinfo(title, msg)

    def get_args(self):
        """
        function that retrieves all data from the entries of the GUI
        interface. Performs value checks to ensure value entered is
        of the correct type.
        :return:
        """
        self.args.startup = self.entry_startup.get()
        self.args.matrix = self.entry_matrixinput.get()
        self.args.output = self.entry_output.get()
        if self.is_server.get() == 1:
            self.args.server = True
            self.args.local = False
            self.args.ip = self.entry_ip.get()
            try:
                self.args.port = int(self.entry_port.get())
            except ValueError:
                if self.entry_port.get() == "":
                    self.display_msg("Error", "No Values passed for port number.")
                else:
                    self.display_msg("Invalid Numeric",
                                     "The values you entered for port number is not a valid literal.")
                return False
            self.args.scenario = self.entry_scenario.get()
            self.args.windows = self.support_windows.get()
            self.args.windows_binary = self.entry_winbin.get()
            self.args.windows_exe = self.entry_winexe.get()
            self.args.linux = self.support_linux.get()
            self.args.linux_binary = self.entry_linbin.get()
            self.args.linux_exe = self.entry_linexe.get()
        else:
            self.args.server = False
            self.args.local = True
            self.args.local_exe = self.entry_lexe.get()
            try:
                self.args.local_threads = int(self.entry_lthreads.get())
            except ValueError:
                if self.entry_lthreads.get() == "":
                    self.args.local_threads = 1
                else:
                    self.display_msg("Invalid Numeric", "The value you entered for number of threads is not valid.")
                    return False
        try:
            self.args.dp_range[0] = 0
            self.args.dp_range[1] = 0
            if self.entry_dp_min.get() != "":
                self.args.dp_range[0] = int(self.entry_dp_min.get())
            if self.entry_dp_max.get() != "":
                self.args.dp_range[1] = int(self.entry_dp_max.get())
        except ValueError:
            self.display_msg("Invalid Numeric", "The values you entered for dp min and max are not valid literals.")
            return False
        try:
            self.args.runs = int(self.entry_runs.get())
        except ValueError:
            if self.entry_runs.get() == "":
                self.args.runs = 1
            else:
                self.display_msg("Invalid Numeric", "The values you entered for number of runs is not valid.")
                return False
        self.args.variables = self.entry_variable.get()
        if self.entry_run_variable.get() == '':
            self.args.run_variable = "run_seed"
        else:
            self.args.run_variable = self.entry_run_variable.get()

        log = self.process_args(self.args)
        if log.error_flag:
            self.display_msg("Error", log.error_string)
            return False
        elif log.notice_string:
            self.display_msg("Notice", log.notice_string)
            return True
        else:
            return True
        # if n log.error_flag:
        #     if log.notice_flag:
        #         self.display_msg("Notice", log.notice_string)
        #     return True
        # elif :
        #     self.display_msg("Error", log.error_string)
        #     return False

    def prelaunch_mgr(self):
        """
        Prelaunch_mgr initializes a thread that will maintain the initialized manager based
        on passed ParseSpace member variable.
        :return:
        """
        try:
            self.stream.kill_event.clear()
            if not self.start_manager_lock:
                if self.get_args():
                    manager_thread = threading.Thread(target=self.launch_mgr)
                    manager_thread.start()
        except RuntimeError:
            self.stream.write_terminal("Runtime error found. Thread not created.")

    def launch_mgr(self):
        """
        Launches manager based on ParseSpace member variable data.
        :return:
        """
        self.start_manager_lock = True
        if self.args.server:
            server_thread = threading.Thread(target=self.gen_server_mgr, args=(self.args, self.stream,))
            server_thread.start()
            self.stream.ready_event.wait()
            self.stream.ready_event.clear()
            msg_notification = messagebox.askyesno(title="Server start",
                                                   message="Connect all receiving clients and select \"Yes\" to "
                                                           "begin or \"No\" to cancel.")
            if msg_notification:
                self.stream.start_event.set()
            else:
                self.stream.cancel_event.set()
            server_thread.join()
        else:
            local_thread = threading.Thread(target=self.gen_local_mgr, args=(self.args, self.stream,))
            local_thread.start()
            local_thread.join()
        self.start_manager_lock = False

    def open_terminal(self):
        self.terminal_window.deiconify()

    def close_terminal(self):
        self.terminal_window.iconify()

    def update_terminal(self):
        while not self.update_event.is_set():
            self.text_box.delete('1.0', END)
            self.text_box.insert(INSERT, self.stream.get_output_string())
            time.sleep(2)

    def prep_create_script(self):
        """
        helper function that ensures that all data to be written
        to a script file is up to date and correct.
        :return:
        """
        if self.get_args():
            self.gen_script(self.args)

    def load_script(self):
        """
        Parses a given config files and loads them in the correct entry in the
        GUI interface.
        :return:
        """
        print("Loading script")
        file_path = filedialog.askopenfilename(title="Chugger script file",
                                               filetypes=[("batch scripts", "*.bat"),
                                                          ("shell scripts", "*.sh")])
        if file_path == '':  # askopenfilename returns an empty string if the user cancels the selection
            print("No script selected.")
        else:
            with open(file_path, "r") as script_file:
                arguments = deque(shlex.split(script_file.readline()))
                while len(arguments) != 0:
                    args = arguments.popleft()
                    if args == self.AFSIM_INPUT:
                        self.insert_entry_text(self.entry_startup, arguments.popleft())
                    elif args == self.MATRIX_INPUT:
                        self.insert_entry_text(self.entry_matrixinput, arguments.popleft())
                    elif args == self.OUTPUT_FOLDER:
                        self.insert_entry_text(self.entry_output, arguments.popleft())
                    elif args == self.RUNS:
                        self.insert_entry_text(self.entry_runs, arguments.popleft())
                    elif args == self.DP_RANGE:
                        self.insert_entry_text(self.entry_dp_min, arguments.popleft())
                        self.insert_entry_text(self.entry_dp_max, arguments.popleft())
                    elif args == self.VARIABLES:
                        self.insert_entry_text(self.entry_variable, arguments.popleft())
                    elif args == self.RUN_VARIABLE_NAME:
                        self.insert_entry_text(self.entry_run_variable, arguments.popleft())
                    elif args == self.SERVER_MANAGER:
                        self.is_server.set(1)
                        self.radio_server = 1
                        self.display_server_options()
                    elif args == self.IP_ADDRESS:
                        self.insert_entry_text(self.entry_ip, arguments.popleft())
                    elif args == self.PORT_NUMBER:
                        self.insert_entry_text(self.entry_port, arguments.popleft())
                    elif args == self.SCENARIO_FOLDER:
                        self.insert_entry_text(self.entry_scenario, arguments.popleft())
                    elif args == self.LINUX_SUPPORT:
                        self.support_linux.set(1)
                        self.display_platform_options()
                    elif args == self.LINUX_EXEC_FILE:
                        self.insert_entry_text(self.entry_linexe, arguments.popleft())
                    elif args == self.LINUX_BINARY_FOLDER:
                        self.insert_entry_text(self.entry_linbin, arguments.popleft())
                    elif args == self.WINDOWS_SUPPORT:
                        self.support_windows.set(1)
                        self.display_platform_options()
                    elif args == self.WINDOWS_EXEC_FILE:
                        self.insert_entry_text(self.entry_winexe, arguments.popleft())
                    elif args == self.WINDOWS_BINARY_FOLDER:
                        self.insert_entry_text(self.entry_winbin, arguments.popleft())
                    elif args == self.LOCAL_MANAGER:
                        self.radio_local = 1
                        self.is_server.set(0)
                        self.display_local_options()
                    elif args == self.LOCAL_THREADS:
                        self.insert_entry_text(self.entry_lthreads, arguments.popleft())
                    elif args == self.LOCAL_EXEC:
                        self.insert_entry_text(self.entry_lexe, arguments.popleft())
            print("finished loading script")

    def close_manager(self):
        print("Closing Gui Manager")
        self.update_event.set()
        self.stream.kill_event.set()
        for thread in self.thread_list:
            thread.join()
        print("All threads joined")
        self.top_window.quit()
        self.top_window.destroy()

    def kill_processes(self):
        if self.start_manager_lock:
            self.stream.kill_event.set()
            self.start_manager_lock = False
        else:
            self.stream.write_terminal("No active managers")