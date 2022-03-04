const express = require('express');
const app = express();
const port = 3000;

app.get('/', (req, res) => res.send('Hello World!'));

app.listen(port, () => console.log(`Example app listening at http://localhost:${port}`));

const Discord = require('discord.js');
require('dotenv').config()
const client = new Discord.Client({ intents : 32767 });
const feed = 'https://ytviewer.ga/article.txt';
var cache = '';
var filereadstatus = '';
const fs = require('fs');

const imagearray = [
  'https://ichef.bbci.co.uk/images/ic/240xn/p0br0w53.jpg',
  'https://ichef.bbci.co.uk/images/ic/240xn/p0bqzy7l.jpg',
  'https://ichef.bbci.co.uk/images/ic/240xn/p0br2pbx.jpg',
  'https://ichef.bbci.co.uk/images/ic/240xn/p0bqzxhy.jpg',
  'https://ichef.bbci.co.uk/images/ic/240xn/p0br0rgq.jpg',
  'https://ichef.bbci.co.uk/images/ic/240xn/p0bqvc9q.jpg',
  'https://ichef.bbci.co.uk/images/ic/240xn/p0bqxxlj.jpg',
  'https://ichef.bbci.co.uk/images/ic/240xn/p0bqxz2l.jpg',
  'https://ichef.bbci.co.uk/images/ic/240xn/p0bqxlhf.jpg',
  'https://ichef.bbci.co.uk/live-experience/cps/240/cpsprodpb/1214B/production/_123495047_gettyimages-1238894067.jpg',
  'https://ichef.bbci.co.uk/live-experience/cps/240/cpsprodpb/70E2/production/_123489882_mediaitem123485240.jpg'
]

// write cache to file
function writeCache(cache) {
    fs.writeFile('./cache.txt', cache, function(err) {
        if(err) {
            return console.log(err);
        }
        console.log("The file was saved!");
    });
}

function readCache() {
    fs.readFile('./cache.txt', function(err, data) {
        if(err) {
            return console.log(err);
        }
        cache = data;
    });
}

              function writetheTitleCache(newtitle) {
                  fs.readFile('./titlecache.txt', function(err, data) {
                      if(err) {
                          return console.log(err);
                      }
                      cachetitle = data + '\n' + newtitle;
                      fs.writeFile('./titlecache.txt', cachetitle, function(err) {
                        if(err) {
                            return console.log(err);
                        }
                        console.log(cachetitle + "was saved");
                    });
                  });
              }


const {decode} = require('html-entities');

client.on('ready', () => {
  client.user.setActivity('STARTING..');
  console.log(`Logged in as ${client.user.tag}!`);
  setInterval(function() {
    console.log('pulling new info');
    var request = require('request');
    request.get(feed, function (error, response, body) {
        if (!error && response.statusCode == 200) {
            var feed = body;
            var strippedfeed = feed.replaceAll(/<{1}[^<>]{1,}>{1}/g,"");
            
            // for every line break, add a new entry to array
            var arr = strippedfeed.split("\n");
            readCache()
            if (strippedfeed == cache) {
              console.log("No new feed");
              return;
            } else {
              cache = strippedfeed;
              writeCache(cache);
              
            }
            // console.log(arr);
            
            // for every entry in array, check if it starts with title: and remove it
            // then add it to the title array
            var titlearray = [];
            for (var i = 0; i < arr.length; i++) {
                if (arr[i].startsWith("title:")) {
                    titlearray.push(arr[i].replace("title: ", ""));
                }
            }
  
            // for every entry in array, check if it starts with paragraph: and remove it
            // then add it to the paragraph array
            var paragrapharray = [];
            for (var i = 0; i < arr.length; i++) {
                if (arr[i].startsWith("paragraph:")) {
                    paragrapharray.push(arr[i].replace("paragraph: ", ""));
                    paragrapharray.push(" ");
                }

            }
          var title = titlearray.join("\n");

              if (title.includes('Biden') || title.includes('US') || title.includes('United States')) {
                var hasBiden = 'true';
              } else {
                var hasBiden = 'false';
              }

              if (title.includes('Boris') || title.includes('UK') || title.includes('United Kingdom') || title.includes('England')) {
                var hasBoris = 'true';
              } else {
                var hasBoris = 'false';
              }

              if (hasBoris = 'true') {
                var URL = 'https://ichef.bbci.co.uk/images/ic/240xn/p0br2mmw.jpg';
              } else if (hasBiden = 'true') {
                var URL = 'https://ichef.bbci.co.uk/images/ic/240xn/p0br3dw3.jpg';
              } else {
                var random = Math.floor(Math.random() * imagearray.length);
                var URL = imagearray[random];
              }


            // convert title array to string

            fs.readFile('./titlecache.txt', function(err, data) {
              if(err) {
                console.log(err);
                return;
              }
              console.log('DID IT WORK LMAO????????: ' + data.includes(title))
               if (data.includes(title)) {
                console.log('failed as already in file');
                return;
              } else {
                writetheTitleCache(title);
                console.log(title);
                  console.log(paragrapharray);
                  // convert paragraph array to string with newlines
                  var paragraph = paragrapharray.join("\n");
      
                  var decodedtitle = decode(title);
                  var slicedtitle = decodedtitle.slice(8, decodedtitle.length);
                  var embed = new Discord.MessageEmbed()
                  .setTitle(slicedtitle)
                  .setURL('https://www.bbc.com/news/live/world-europe-60582327')
                  .setColor('#ff2d2d')
                  .setThumbnail(URL)
                  .setFooter('BREAKING NEWS')
                  .setTimestamp()
                  .setDescription(decode(paragraph));
                  client.user.setPresence({
                    status: 'dnd',
                  });
                  client.user.setActivity(slicedtitle, {
                    type: 'WATCHING'
                  });
                  client.channels.cache.get('946504863154589718').send('**' + slicedtitle + '** | @everyone'); 
                  client.channels.cache.get('946504863154589718').send({ embeds: [embed] });
              } 
            });
            
        }
    });
  }, 20000);

});


client.login(process.env.TOKEN);
