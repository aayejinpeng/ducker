#include<string>
#include<iostream>
#include<vector>
#include<cstdlib>
#include<cstdio>
#include<fstream>
#include<sstream>
#include"ducker.h"
using namespace std;

/*
  attach      Attach local standard input, output, and error streams to a running container
  build       Build an image from a Dockerfile
  commit      Create a new image from a container's changes
  cp          Copy files/folders between a container and the local filesystem
  create      Create a new container
  diff        Inspect changes to files or directories on a container's filesystem
  events      Get real time events from the server
  exec        Run a command in a running container
  export      Export a container's filesystem as a tar archive
  history     Show the history of an image
  images      List images
  import      Import the contents from a tarball to create a filesystem image
  info        Display system-wide information
  inspect     Return low-level information on Docker objects
  kill        Kill one or more running containers
  load        Load an image from a tar archive or STDIN
  login       Log in to a Docker registry
  logout      Log out from a Docker registry
  logs        Fetch the logs of a container
  pause       Pause all processes within one or more containers
  port        List port mappings or a specific mapping for the container
  ps          List containers
  pull        Pull an image or a repository from a registry
  push        Push an image or a repository to a registry
  rename      Rename a container
  restart     Restart one or more containers
  rm          Remove one or more containers
  rmi         Remove one or more images
  run         Run a command in a new container
  save        Save one or more images to a tar archive (streamed to STDOUT by default)
  search      Search the Docker Hub for images
  start       Start one or more stopped containers
  stats       Display a live stream of container(s) resource usage statistics
  stop        Stop one or more running containers
  tag         Create a tag TARGET_IMAGE that refers to SOURCE_IMAGE
  top         Display the running processes of a container
  unpause     Unpause all processes within one or more containers
  update      Update configuration of one or more containers
  version     Show the Docker version information
  wait        Block until one or more containers stop, then print their exit codes
*/
struct command
{
  string name;
  string info;
  string useage;
  void* fuc;
};
int container_conter; //最大255，不能申请超过255个，小bug可以操作
vector<command>commands_list = {
  {"help","Display ducker's commands infomation","",(void *)help},
  {"images","List images","",(void *)images},
  {"crate","Create a new container","crate [image] [container]",(void *)crate},
  {"rm","Remove one container","rm [container]",(void *)rm}
};
void help()
{
  for (size_t i = 0; i < commands_list.size(); i++)
  {
    cout<< commands_list[i].name << '\t' << commands_list[i].info << endl;
  }
  
}
void images()
{
  system("ls images");
}
void crate(int argc, char *argv[])
{
  if (argc == 4)
  {
    string image = argv[2];
    string container = argv[3];
    string tmpstr;
    // overlay文件系统
    tmpstr = "mkdir containers/"+container;
    if(system(tmpstr.c_str())) return;
    tmpstr = "mkdir containers/"+container+"/rwlayer";
    if(system(tmpstr.c_str())) return;
    tmpstr = "mkdir containers/"+container+"/overlaywork";
    if(system(tmpstr.c_str())) return;
    tmpstr = "mkdir containers/"+container+"/merged";
    if(system(tmpstr.c_str())) return;
    tmpstr = "sudo mount -t overlay overlay -o lowerdir=images/"+image+",upperdir=containers/"+container+"/rwlayer,workdir=containers/"+container+"/overlaywork containers/"+container+"/merged";
    if(system(tmpstr.c_str())) return;
    // dns配置
    tmpstr = "cp /etc/resolv.conf containers/"+container+"/merged/etc/resolv.conf";
    if(system(tmpstr.c_str())) return;

    // init.sh
    stringstream ss;
    ss << "mount -t proc none /proc \n";
    ss << "mount -t sysfs none /sys \n";
    ss << "mount -t tmpfs none /tmp \n";
    ss << "source /etc/profile \n";
    ss << "export PS1=\"(" << container << " container) \\u:\\w\\$ \" \n";
    ss << "ip addr add 10.0.3." << (container_conter+2) <<"/24 dev vethc-"<< (container_conter)<<" \n";
    ss << "ip link set vethc-"<< (container_conter)<<" up \n";
    ss << "ip route add default via 10.0.3.1 \n";
    ss << "bash -l\n";
    fstream file("./containers/"+container+"/merged/bin/container-init.sh",ios::out);
    file << ss.str();
    container_conter++;//bug
  }else
  {
    cout << "crate [image] [container]" << endl;
  }
}
void rm(int argc, char *argv[])
{
  if (argc == 3)
  {
    string container = argv[2];
    string tmpstr;

    tmpstr = "sudo umount containers/"+container+"/merged";
    if(system(tmpstr.c_str())) return;
    tmpstr = "sudo rm -r containers/"+container;
    if(system(tmpstr.c_str())) return;
  }else
  {
    cout << "rm [container]" << endl;
  }
}
void ducker_init()
{
  fstream info_file("./ducker.info",ios::in);
  string info_title;
  if (!info_file.fail())
  {
    while(info_file>>info_title)
    {
      if (info_title == "container_conter")
      {
        info_file >> container_conter;
      }
    }
  }
}
void ducker_info_save()
{
  fstream info_file("./ducker.info");
  info_file << "container_conter " << container_conter << endl;
}
int main(int argc, char *argv[])
{
  ducker_init();
  if (argc == 1)
  {
    help();
  }
  else if(argc >= 2)
  {
    string command = argv[1];
    if (command == "help")
    {
      help();
    }else if (command == "images")
    {
      images();
    }else if (command == "crate")
    { 
      crate(argc,argv);
    }
    else if (command == "rm")
    { 
      rm(argc,argv);
    }
  }
  ducker_info_save();
}