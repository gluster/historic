from Gluster.GFrontEnd import dialog
from optparse import OptionParser

dlg = dialog.Dialog ()

class GArgp:
    Args = [];
    parser = None;
    Opts = {};

    Options = {

#'varname' : [  ['-v','--varname'],
#               'string',
#	       'defaultstring',
#	       'This is a sample example of registry entry' 
#	    ],
#	       
#'num' :     [ ['-n','--number'],
#               'int',
#	       '3',
#	       'This is another example of registry entry' 
#	    ]
        };

    def __init__ (self):
        self.parser = OptionParser ();

    def GetRawOptConf (self):
        return self.Options;

    def SetRawOptConf (self, reg):
        self.Options = reg;
        return ;

    def AddRawOptConf (self, reg):
        for k,v in reg.iteritems ():
            self.Options[k] = v;
            try:
                if (v[4] != None):
                    cbk = v[4];
            except IndexError:
                cbk = self.SetValCallback;

            if (len (v[0]) == 1):
                self.parser.add_option (v[0][0], action = "callback",
                               callback = cbk, dest = k,
                               default = v[2], help = v[3], type = v[1],
                               nargs = 1);
            if (len (v[0]) == 2):
                self.parser.add_option (v[0][0], v[0][1], action = "callback",
                               callback = self.SetValCallback, dest = k,
                               default = v[2], help = v[3], type = v[1],
                               nargs = 1);
        return ;

    def GetOpt (self, option):
        return self.Opts [option];
    
    def SetOptCB (self, option, opt_str, value, parser):
        self.Opts [option.dest] = value;
                  
    def AddOpt (self, opts):
        for opt in opts:
            try:
                _cbk = opt['callback'];
            except KeyError:
                _cbk = self.SetOptCB;
            try:
                _nargs = opt['nargs'];
            except KeyError:
                _nargs = 1;
            try:
                self.Opts [opt['dest']] = opt['default'];
            except KeyError:
                True
            for key in opt['keys']:
                self.parser.add_option (key, action = "callback",
                                        callback =  _cbk,
                                        help = opt['help'],
                                        dest = opt['dest'],
                                        type = opt['type'],
                                        default = opt['default'],
                                        nargs = _nargs);
        return ;

    def AddVar (self, reg):
        return ;

    def AddArgs (self, var):
        self.Args.append ( var);
        return ;

    def GetArgs (self):
        return self.Args;

    def GetValueInteractive (self, var):
        msg = self.GetDescription (var);
        newval =  dlg.inputbox (msg);
        return newval;

    def GetValue (self, var):
        return self.Options[var][2];


    def SetValue (self, var, val="__default_gluster_arg__"):
        if val in ["__default_gluster_arg__"]:
            val = dlg.inputbox ("Enter value for " + var);
        self.Options[var][2] = val;
        return ;
    

    def GetDescription (self, var):
        try:
            msg = self.Options[var][3];
        except KeyError:
            msg =  var + " " + self.Options[var][1];
        return msg;

    def SetDescription (self, var, val):
        self.Options[var][3] = val;
        return ;


    def SetValCallback (self, option, opt_str, value, parser):
        self.SetValue (option.dest, value);
        return ;

    def Process (self):
        (options, args) = self.parser.parse_args ();
        self.Args = [];
        for i in args:
            self.AddArgs (i)
        return ;

_G = GArgp ();
