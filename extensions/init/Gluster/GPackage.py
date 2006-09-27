import GRegistry, os, sys;
import GArgs;

AllPkgs = [];

class GPkg:
    template = {};
    def __init__ (self, tmpl):
        
        for k,v in tmpl.iteritems ():
            self.template[k] = v;

        AllPkgs.append (self);
        return ;
    
    def Extract (self):
        try:
            for tgz, path in self.template['tgz'].iteritems ():
                try:
                    os.system ("mkdir -p " + path );
                    os.system ("tar -C " + path + " -xvzf " + os.path.dirname (sys.argv[0]) + "/" + tgz);
                except OSError:
                    break ;
        except KeyError:
            return -1;
        return 0;
    
    def Install (self):
        try:
            print "Author: " + self.template['author'];
        except KeyError:
            self.template['author'] = '';
        self.Extract ();
        return 0;

    def Desc (self):
        try:
            return self.template['desc'];
        except KeyError:
            return self.Name ();
        return None;
    
    def Name (self):
        try:
            return self.template['name'];
        except KeyError:
            return os.path.basename (sys.argv[0]);
        return None;
    
    def Uninstall (self):
        return 0;
    
    def Load (self):
        return 0;
    
