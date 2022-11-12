大一下做的一个简单的扫雷游戏

用的是之前的电脑做的，现在的电脑上只有这一个代码文件。毕竟是第一次自己做的一个小项目，还是挺有纪念意义的，为了防止电脑硬盘坏的意外情况考虑了一下还是上传一下Github。

用的是Win32平台的API实现的界面，界面的设计上参考了经典的扫雷游戏。

![](./Screenshot.png)

实现了简单的排行榜的功能，排行榜内容保存在与可运行文件相同路径下的RankingList.txt中。

游戏性方面作出了一点创新，具体表现为在已翻开的块中会随机生成一个“item”块，点击后就可以获得一个“道具”，在点击到雷块后可以消耗一个道具来使游戏继续进行。
