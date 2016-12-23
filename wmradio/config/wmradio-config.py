#!/usr/bin/env python

import gobject, gtk, time, os, string
from gettext import gettext as _
from gettext import bindtextdomain, textdomain
from locale import setlocale, LC_ALL
import os

PREFIX = "/usr"
LOCALEDIR = "%s/share/locale" % (PREFIX)
PACKAGE = "wmradio-config.py"
SKINDIR = "%s/lib/wmradio" % (PREFIX)
FREQMIN=87.5
FREQMAX=108.0
XOSD = "undef " == "define"

class inifile:
    def __init__(self,afilename):
        self.sections = {};
        try:
            self.load(afilename)
        except IOError:
            pass
    def load(self,afilename):
        self.filename = afilename
        f = open(afilename,'r');
        lines = f.readlines();
        section = "config";
        self.sections = {};
        self.sections[section] = {}
        for line in lines:
            if line[:1] == ';' or line[:1] == '#' : line = '';
            if len(line)> 0:
                if line[0] == '[':
                    e = string.find(line,']');
                    section = string.lower(line[1:e])
                    self.sections[section] = {}
                else:
                    eq = string.find(line,"=")
                    if eq >= 0 :
                        variable = string.strip(string.lower(line[:eq]))
                        value = string.strip(line[eq+1:])
                        e = string.find(value,'\n')
                        if e>=0 :
                            value = value[0:e]
                        self.sections[section][variable] = value
        f.close();
    def save(self):
        f = file(self.filename,"w+")
        for section in self.sections.keys():
            f.write('[%s]\n' % (section))
            for value in self.sections[section].keys():
                f.write("%s=%s\n" % (value,self.sections[section][value]))
        f.close()
    def save_as(self,newfile):
        self.filename = newfile
        self.save()
    def get_variable(self,sec,var,default=""):
        sec = string.lower(sec)
        var = string.lower(var)
        if not self.sections.has_key(sec) : return default
        if not self.sections[sec].has_key(var) : return default
        return self.sections[sec][var]
    def get_int_variable(self,sec,var,default=0):
        try:
            val = self.get_variable(sec,var)
            if val == "" : return default
            return string.atoi(val)
        except: return 0;
    def set_variable(self,sec,var,val):
        if not self.sections.has_key(string.lower(sec)):
            self.sections[string.lower(sec)] = {}
        self.sections[string.lower(sec)][string.lower(var)] = val
    def set_int_variable(self,sec,var,val):
        self.set_variable(sec,var,"%i" % (val))

# --------------------------------------------------------------------

class WmRadioConfig:
    def __init__(self):
        self.g_window = None;
        self.g_quit_button = None;
        self.g_device = None;
        self.g_ok_button = None;
        self.g_cancel_button = None;
        self.g_pressed = []
        self.g_new_freq = None;
        self.g_new_name = None;
        self.g_delete_button = None;
        self.g_osd = None;
        self.g_osd_font = None;
        self.g_osd_color = None;
        self.g_osd_position = None;
        self.g_osd_shadow = None;
        self.g_osd_timeout = None;
        self.g_start_muted = None;
        self.g_dont_quit = None;
    def quit(self, *args):
	self.g_window.destroy()
    def one_station_to_ini(self,model,path,iter,data):
        line = self.g_jobs.get_value(iter,0)
        i = string.find(line,"=")
        if i > -1:
            name = line[:i]
            freq = line[i+1:]
            self.cfg.set_variable("names", name, freq)
    def station_to_ini(self):
        if self.cfg.sections.has_key("names") : del self.cfg.sections["names"]
        self.g_jobs.foreach(self.one_station_to_ini,0)
    def on_osd_changed(self, *args):
        sensitive = self.g_osd.get_active()
        self.g_osd_font.set_sensitive(sensitive)
        self.g_osd_color.set_sensitive(sensitive)
        self.g_osd_position.set_sensitive(sensitive)
        self.g_osd_shadow.set_sensitive(sensitive)
        self.g_osd_timeout.set_sensitive(sensitive)
    def on_apply_click(self, *args):
        self.store_config()
    def on_ok_click(self, *args):
        self.store_config()
        self.g_window.destroy()
    def store_config(self):
        self.cfg.set_variable("config","device",self.g_device.get_text())
        self.cfg.set_variable("config","skin",self.g_skin.entry.get_text())
        for i in range(6):
            self.cfg.set_variable("config",
                                  "preset%i" % i ,
                                  "%.2f" % self.g_pressed[i].get_value())
        self.station_to_ini()
        self.cfg.set_int_variable("config","osd", self.g_osd.get_active())
        self.cfg.set_variable("config","osd-font",self.g_osd_font.get_text())
        self.cfg.set_variable("config","osd-color", self.g_osd_color.get_text())
        self.cfg.set_int_variable("config","osd-position",self.g_osd_position.get_value())
        self.cfg.set_int_variable("config","osd-shadow-offset", self.g_osd_shadow.get_value())
        self.cfg.set_int_variable("config","osd-timeout",self.g_osd_timeout.get_value())
        self.cfg.set_int_variable("config","start-muted",self.g_start_muted.get_active())
        self.cfg.set_int_variable("config","dont-quit-mode",self.g_dont_quit.get_active())
        self.cfg.save()
        self.refresh_running_radio()
    def destroy(self, *args):
	gtk.main_quit()
    def load_config(self):
        self.cfg = inifile("%s/.wmradio" % (os.environ['HOME']))
        self.g_device.set_text(self.cfg.get_variable("config","device"))
        self.g_skin.entry.set_text(self.cfg.get_variable("config","skin"))
        if self.cfg.get_variable("config","start-muted") == "1":
            self.g_start_muted.set_active(gtk.TRUE)
        if self.cfg.get_variable("config","dont-quit-mode") == "1":
            self.g_dont_quit.set_active(gtk.TRUE)
        for i in range(6):
            try:
                self.g_pressed[i].set_value(string.atof(self.cfg.get_variable("config","preset%i" % i )))
            except:
                pass
        if self.cfg.sections.has_key("names"):
            for name in self.cfg.sections["names"].keys():
                item = self.g_jobs.append()
                self.g_jobs.set(item,0,
                                "%s=%s" %
                                (name,self.cfg.get_variable("names",name))
                                )
        if self.cfg.get_variable("config","osd") == "1":
            self.g_osd.set_active(gtk.TRUE)
        self.g_osd_font.set_text(self.cfg.get_variable("config",
                                                       "osd-font"))
        self.g_osd_color.set_text(self.cfg.get_variable("config",
                                                        "osd-color"))
        self.g_osd_position.set_value(self.cfg.get_int_variable("config",
                                                                "osd-position"))
        self.g_osd_shadow.set_value(self.cfg.get_int_variable("config",
                                                              "osd-shadow-offset"))
        self.g_osd_timeout.set_value(self.cfg.get_int_variable("config",
                                                               "osd-timeout"))
        self.on_osd_changed(None)
    def delete_selected_name(self,model,path,iter,data):
        model.remove(iter)
    def on_delete(self,widget):
        self.g_tree_view.get_selection().selected_foreach(self.delete_selected_name,0)
        self.on_selection_changed(widget)
    def on_add_name(self,widget):
        self.station_alredy_exist = 0
        self.g_jobs.foreach(self.stations_exist,self.g_new_name.get_text())
        if not self.station_alredy_exist:
            item = self.g_jobs.append()
            self.g_jobs.set(item,
                            0,
                            "%s=%.2f" %
                            (self.g_new_name.get_text(),
                             self.g_new_freq.get_value()))
    def activate_delete_button(self,model,path,iter,data):
        self.g_delete_button.set_sensitive(gtk.TRUE)
    def on_selection_changed(self,widget=None):
        self.g_delete_button.set_sensitive(gtk.FALSE)
        self.g_tree_view.get_selection().selected_foreach(self.activate_delete_button,0)
    def on_name_changed(self,widget):
        self.g_add_button.set_sensitive( len(string.strip(self.g_new_name.get_text())) > 0 )
    def refresh_running_radio(self):
        pipe = "/tmp/wmradio_%i" % os.getuid()
        if os.path.exists(pipe):
            f = open(pipe,"w+")
            f.write("READ_CONFIG\n")
    def stations_exist(self,model,path,iter,text):
        line = self.g_jobs.get_value(iter,0)
        i = string.find(line,"=")
        if i > -1:
            name = line[:i]
            if string.lower(name) == string.lower(text):
                self.g_jobs.set(iter,
                                0,
                                "%s=%.2f" %
                                (self.g_new_name.get_text(),
                                 self.g_new_freq.get_value()))
                self.station_alredy_exist = 1
    def check_that_station_exist():
        self.station_alredy_exist = 0
        self.g_jobs.foreach(self.stations_exist,0)
    def fill_combo(self):
        files = os.listdir(SKINDIR)
        skins = []
        for file in files:
            if file[-5:] == ".skin":
                skins.append(file)
        if len(skins): self.g_skin.set_popdown_strings(skins)
    def frame1(self):
        frame = gtk.Frame(_("Base config"))
        frame.set_border_width(5)
        table = gtk.Table(rows = 2, columns = 5, homogeneous=gtk.FALSE)
        table.set_border_width(5)
        table.set_col_spacings(5)
        frame.add(table)
        lbl = gtk.Label(_("Device"))
        lbl.set_alignment(xalign=0.0,yalign=0.5)
        table.attach(lbl,0,1,0,1)
        self.g_device = gtk.Entry()
        table.attach(self.g_device,1,2,0,1)

        lbl = gtk.Label(_("Skin"))
        lbl.set_alignment(xalign=0.0,yalign=0.5)
        table.attach(lbl,0,1,1,2)
        self.g_skin = gtk.Combo()
        self.fill_combo()
        table.attach(self.g_skin,1,2,1,2)

        self.g_start_muted =  gtk.CheckButton(_("Start muted"))
        table.attach(self.g_start_muted,0,2,2,3)

        self.g_dont_quit = gtk.CheckButton(_("Don't quit mode"))
        table.attach(self.g_dont_quit,0,2,3,4)

        return frame
    def frame2(self):
        frame = gtk.Frame(_("Stations"))
        frame.set_border_width(5)
        table = gtk.Table(rows = 2, columns = 6, homogeneous=gtk.FALSE)
        table.set_border_width(5)
        table.set_col_spacings(5)
        frame.add(table)
        for i in range(6):
            lbl = gtk.Label(_("Station %i") % (i+1))
            lbl.set_alignment(xalign=0.0,yalign=0.5)
            table.attach(lbl,0,1,i,i+1)
            self.g_pressed.append( gtk.SpinButton(gtk.Adjustment(FREQMIN,
                                                                 FREQMIN,
                                                                 FREQMAX,
                                                                 0.1,
                                                                 1,
                                                                 1
                                                                 ),
                                                  0.0,
                                                  2) );
            table.attach(self.g_pressed[i],1,2,i,i+1)
        return frame
    def frame3(self):
        frame = gtk.Frame(_("Stations names"))
        frame.set_border_width(5)
        table = gtk.Table(rows = 2, columns = 3, homogeneous=gtk.FALSE)
        table.set_border_width(5)
        table.set_col_spacings(5)
        frame.add(table)
        box = gtk.HBox(gtk.FALSE,2)
        table.attach(box,0,1,0,1,yoptions=0)
        lbl = gtk.Label(_("Name"))
        box.pack_start(lbl,gtk.TRUE,gtk.TRUE,0)
        self.g_new_name = gtk.Entry(max=3);
        self.g_new_name.connect("changed",self.on_name_changed)
        box.pack_start(self.g_new_name,gtk.TRUE,gtk.TRUE,0)
        lbl = gtk.Label(_("Frequency"))
        box.pack_start(lbl,gtk.TRUE,gtk.TRUE,0)
        self.g_new_freq = gtk.SpinButton(gtk.Adjustment(FREQMIN,
                                                        FREQMIN,
                                                        FREQMAX,
                                                        0.1,
                                                        1,
                                                        1),
                                         0.0,
                                         2)
        box.pack_start(self.g_new_freq,gtk.TRUE,gtk.TRUE,0)
        self.g_add_button = gtk.Button(_("Add"))
        box.pack_start(self.g_add_button,gtk.TRUE,gtk.TRUE,0)
        self.g_add_button.connect("clicked",self.on_add_name)
        self.g_add_button.set_sensitive(gtk.FALSE)

        scrolled_window = gtk.ScrolledWindow()
        scrolled_window.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.g_jobs = gtk.ListStore(gobject.TYPE_STRING)
        self.g_tree_view = gtk.TreeView(self.g_jobs)
        scrolled_window.add_with_viewport (self.g_tree_view)
        cell = gtk.CellRendererText()
        column = gtk.TreeViewColumn(_("Name=Frequency"), cell, text=0)
        self.g_tree_view.append_column(column)
        table.attach(scrolled_window,0,1,1,2,yoptions=gtk.EXPAND|gtk.FILL)
        self.g_tree_view.get_selection().connect("changed",self.on_selection_changed)

        self.g_delete_button = gtk.Button(_("Delete"))
        self.g_delete_button.connect("clicked",self.on_delete)
        self.g_delete_button.set_sensitive(gtk.FALSE)

        table.attach(self.g_delete_button,0,1,2,3,yoptions=0)
        return frame
    def frame4(self):
        vbox = gtk.VBox(gtk.FALSE,2)
        frame = gtk.Frame(_("osd"))
        frame.set_border_width(5)
        vbox.pack_start(frame, gtk.FALSE,gtk.FALSE)
        #vbox.pack_start(gtk.VBox(gtk.FALSE), gtk.TRUE,gtk.TRUE)
        table = gtk.Table(rows = 2, columns = 3, homogeneous=gtk.FALSE)
        table.set_border_width(5)
        table.set_col_spacings(5)
        frame.add(table)

        self.g_osd = gtk.CheckButton(_("Use osd"))
        table.attach(self.g_osd,0,2,0,1)
        self.g_osd.connect("clicked",self.on_osd_changed,None)
        # osd font
        lbl = gtk.Label(_("Font"))
        lbl.set_alignment(xalign=0.0,yalign=0.5)
        table.attach(lbl,0,1,1,2)
        self.g_osd_font = gtk.Entry()
        table.attach(self.g_osd_font,1,2,1,2)
        # osd color
        lbl = gtk.Label(_("Color"))
        lbl.set_alignment(xalign=0.0,yalign=0.5)
        table.attach(lbl,0,1,2,3)
        self.g_osd_color = gtk.Entry()
        table.attach(self.g_osd_color,1,2,2,3)
        # position
        lbl = gtk.Label(_("Position"))
        lbl.set_alignment(xalign=0.0,yalign=0.5)
        table.attach(lbl,0,1,3,4)
        self.g_osd_position = gtk.SpinButton(gtk.Adjustment(0,0,5000,1,5,0))
        table.attach(self.g_osd_position,1,2,3,4)
        # shadow offset
        lbl = gtk.Label(_("Shadow offset"))
        lbl.set_alignment(xalign=0.0,yalign=0.5)
        table.attach(lbl,0,1,4,5)
        self.g_osd_shadow = gtk.SpinButton(gtk.Adjustment(0,0,5000,1,5,0))
        table.attach(self.g_osd_shadow,1,2,4,5)
        # timeout
        lbl = gtk.Label(_("Timeout"))
        lbl.set_alignment(xalign=0.0,yalign=0.5)
        table.attach(lbl,0,1,5,6)
        self.g_osd_timeout = gtk.SpinButton(gtk.Adjustment(0,0,5000,1,5,0))
        table.attach(self.g_osd_timeout,1,2,5,6)

        return vbox
    def create_gui(self):
        setlocale(LC_ALL,"")
        bindtextdomain( PACKAGE, LOCALEDIR )
        textdomain( PACKAGE )
        self.g_window = gobject.new(gtk.Window,
                                    type=gtk.WINDOW_TOPLEVEL,
                                    title="WmRadio Config",
                                    allow_grow=gtk.FALSE,
                                    allow_shrink=gtk.FALSE,
                                    border_width=10)
        self.g_window.connect("destroy", self.destroy)
        vbox = gtk.VBox(gtk.FALSE,2)
        self.g_window.add(vbox)
        self.g_notebook = gtk.Notebook()
        vbox.pack_start(self.g_notebook,gtk.TRUE,gtk.TRUE)

        pagevbox = gtk.VBox(gtk.FALSE,2)
        pagevbox.pack_start(self.frame1(),gtk.TRUE,gtk.TRUE)
        pagevbox.pack_start(self.frame2(),gtk.TRUE,gtk.TRUE)
        self.g_notebook.append_page(pagevbox,
                                    gtk.Label(_("Config")));
        self.g_notebook.append_page(self.frame3(),
                                    gtk.Label(_("Names")));
	lbl = gtk.Label(_("osd"))
        self.g_notebook.append_page(self.frame4(),lbl);
	if not XOSD:
	    lbl.set_sensitive(gtk.FALSE)
	    self.g_notebook.get_nth_page(2).set_sensitive(gtk.FALSE)
        box = gtk.HBox(gtk.TRUE,2)
        vbox.pack_start(box,gtk.FALSE,gtk.FALSE)
        self.g_ok_button = gtk.Button(_("Ok"))
        self.g_ok_button.connect("clicked",self.on_ok_click)
        box.pack_start(self.g_ok_button,gtk.TRUE,gtk.TRUE,0)
        self.g_apply_button = gtk.Button(_("Apply"))
        self.g_apply_button.connect("clicked",self.on_apply_click)
        box.pack_start(self.g_apply_button,gtk.TRUE,gtk.TRUE,0)
        self.g_cancel_button = gtk.Button(_("Cancel"))
        self.g_cancel_button.connect("clicked",self.quit)
        box.pack_start(self.g_cancel_button,gtk.TRUE,gtk.TRUE,0)
        self.g_window.show_all()
    def run(self):
        self.create_gui()
        self.load_config()
        gtk.main()


WmRadioConfig().run()
