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
  {"create","Create a new container","create [image] [container]",(void *)create},
  {"rm","Remove one container","rm [container]",(void *)rm},
  {"exec","Run a command in a running container","exec [container] [command...]",(void *)exec}
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
void create(int argc, char *argv[])
{
  if (argc == 4)
  {
    string image = argv[2];
    string container = argv[3];
    string tmpstr;
    stringstream ss;
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

  // 网络设置
    // 新建网线对vethc-id && vethc-bc-id
    ss.str("");
    ss << "sudo ip link add vethc-" <<container_conter << " type veth peer name vethc-br-" << container_conter;
    system(ss.str().c_str());

    // 新建netns
    ss.str("");
    ss << "sudo ip netns add container-"<< container <<"-netns";
    system(ss.str().c_str());

    //将新建的网线一端放入netns中
    ss.str("");
    ss << "sudo ip link set vethc-"<<container_conter<<" netns container-"<< container <<"-netns ";
    system(ss.str().c_str());

    ss.str("");
    // ip link set dev veth1 master br0
    ss << "sudo ip link set dev vethc-br-"<<container_conter<<" master br0";
    system(ss.str().c_str());

    ss.str("");
    ss << "sudo ip link set vethc-br-"<<container_conter<<" up";
    system(ss.str().c_str());

    // cgroup设置
    ss.str("");
    ss << "sudo cgcreate -g cpu:cpu_share_" << container_conter;
    system(ss.str().c_str());

    ss.str("");
    cout << "Cpu core size(double example: 2.7):" ;
    double coresize = 1;
    cin >> coresize;
    ss << "sudo cgset -r cpu.cfs_quota_us="<< (unsigned long)(coresize*100000) <<" cpu_share_" << container_conter;
    system(ss.str().c_str());

    ss.str("");
    ss << "sudo cgcreate -g memory:mem_share_" << container_conter;
    system(ss.str().c_str());

    ss.str("");
    cout << "memory size(MB)(double example: 1024):" ;
    double memory = 1024*1024;
    cin >> memory;
    ss << "sudo cgset -r memory.limit_in_bytes="<< (unsigned long)(memory*1024*1024) <<" mem_share_" << container_conter;
    system(ss.str().c_str());
    
    // // init.sh
    // ss.str("");
    // ss << "mount -t proc none /proc \n";
    // ss << "mount -t sysfs none /sys \n";
    // ss << "mount -t tmpfs none /tmp \n";
    // ss << "source /etc/profile \n";
    // ss << "export PS1=\"(" << container << " container) \\u:\\w\\$ \" \n";
    // // ss << "ip addr add 10.0.3." << (container_conter+2) <<"/24 dev vethc-"<< (container_conter)<<" \n";
    // // ss << "ip link set vethc-"<< (container_conter)<<" up \n";
    // // ss << "ip route add default via 10.0.3.1 \n";
    // ss << "bash -l\n";
    // fstream file("./containers/"+container+"/merged/bin/container-init.sh",ios::out);
    // file << ss.str();
    // tmpstr = "chmod +x ./containers/"+container+"/merged/bin/container-init.sh";
    // system(tmpstr.c_str());

    cout << "container = " << container << "\nip = 10.0.5." << (container_conter+2) << "/24" << endl;
    fstream id_file("./containers/"+container+"/id",ios::out);
    id_file << container_conter;

    container_conter++;
  }
  else
  {
    cout << "create [image] [container]" << endl;
  }
}
void rm(int argc, char *argv[])
{
  if (argc == 3)
  {
    string container = argv[2];
    string tmpstr;

    tmpstr = "sudo umount containers/"+container+"/merged";
    if(system(tmpstr.c_str())) ;
    tmpstr = "sudo rm -r containers/"+container;
    if(system(tmpstr.c_str())) ;
  }
  else
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
  else
  {
    container_conter = 0;
  }
  
  system("sudo ip link add name br0 type bridge");
  system("sudo ip addr add 10.0.5.1/24 dev br0 ");
  system("sudo ip link set dev br0 up ");
  // system("sudo iptables -t nat -A POSTROUTING -s 10.0.5.0/24 -j MASQUERADE ");
  system("sudo iptables -A FORWARD -o br0 -j ACCEPT ");
  system("sudo iptables -A FORWARD -i br0 -j ACCEPT");
}
void ducker_info_save()
{
  fstream info_file("./ducker.info");
  info_file << "container_conter " << container_conter << endl;
}

void exec(int argc, char *argv[])
{
  if (argc >= 4)
  {
    string container = argv[2];
    string tmpstr;
    stringstream ss;

    int id;
    fstream id_file("./containers/"+container+"/id");
    id_file >> id;

    for (size_t i = 3; i < argc; i++)
    {
      tmpstr += " ";
      tmpstr += argv[i];
    }

    ss.str("");
    // ss << "sudo ip netns exec container-"<<container<<"-netns ip link set dev vethc-"<<id<<" name eth"<<id<<endl;
    system(ss.str().c_str());
    
    ss.str("");
    ss << "sudo ip netns exec container-"<<container<<"-netns ip addr add 10.0.5."<<(id+2)<<"/24 dev vethc-"<< (id)<<endl;
    system(ss.str().c_str());

    ss.str("");
    ss << "sudo ip netns exec container-"<<container<<"-netns ip link set dev vethc-"<<id<< " up" <<endl;
    system(ss.str().c_str());
    ss.str("");
    ss << "mount -t proc none /proc \n";
    ss << "mount -t sysfs none /sys \n";
    ss << "mount -t tmpfs none /tmp \n";
    ss << "source /etc/profile \n";
    ss << "export PS1=\"(" << container << " container) \\u:\\w\\$ \" \n";
    // ss << "ip addr add 10.0.3." << (id+2) <<"/24 dev vethc-"<< (id)<<" \n";
    // ss << "ip link set vethc-"<< (id)<<" up \n";
    ss << "ip route add default via 10.0.5.1 \n";
    ss << tmpstr<< '\n';
    ss << "bash -l\n";
    fstream file("containers/"+container+"/merged/bin/container-exec.sh",ios::out);
    file << ss.str();
    // file.sync();
    file.close();
    tmpstr = "sudo chmod +x ./containers/"+container+"/merged/bin/container-exec.sh";
    system(tmpstr.c_str());
    ss.str("");
    ss << "sudo cgexec -g cpu:cpu_share_"<<id<<" -g memory:mem_share_"<<id<<" ip netns exec container-"<<container<<"-netns unshare -mpfu chroot containers/"<<container<<"/merged env -i HOME=/root TERM=\"$TERM\" container-exec.sh";
    if(system(ss.str().c_str())) return;
  }
  else
  {
    cout << "exec [container] [command...]" << endl;
  }
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
    }
    else if (command == "images")
    {
      images();
    }
    else if (command == "create")
    { 
      create(argc,argv);
    }
    else if (command == "rm")
    { 
      rm(argc,argv);
    }
    else if (command == "exec")
    { 
      exec(argc,argv);
    }
    else
    {
      help();
    }
  }
  ducker_info_save();
}