寻找co-author
========================================

输入数据
========================================
PaperAuthor.csv <br/>
Author.csv <br/>
放在data文件夹下 <br/>

输出数据
========================================
CoAuthor.csv <br/>
<strong>规格: <strong><br/>
按照Author.csv的顺序排序 <br/>
第一行为0 //Author.csv对应的是注释 <br/>
每行第一列为aid ，第2~N列为此aid的co-author 的author id <br/>

跑部分或者所有数据
========================================
部分数据： 将Line 11 的 #define 去掉注释 <br/>
所有数据： 将Line 11 的 #define 前加上注释 <br/>

运行
========================================
编译: <br/>
````
make rebuild
````
后台运行: <br/>
````
make run
````



备注
========================================
每行第一个是aid,后面是co-author id
部分数据跑完可以没问题。



