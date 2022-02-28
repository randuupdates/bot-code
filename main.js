
const Discord = require('discord.js');
require('dotenv').config()
const client = new Discord.Client({ intents : 32767 });
const feed = 'https://ytviewer.ga/article.txt';
var cache = '';
const {decode} = require('html-entities');

client.on('ready', () => {
  console.log(`Logged in as ${client.user.tag}!`);
  setInterval(function() {
    console.log('pulling new info');
    var request = require('request');
    request.get(feed, function (error, response, body) {
        if (!error && response.statusCode == 200) {
            if (cache == "") {
              var feed = body;
              cache = body;
            } else if (body == cache) {
              console.log("No new feed");
              return
            } else {
              var feed = body;
              cache = body;
            }
            

            var strippedfeed = feed.replaceAll(/<{1}[^<>]{1,}>{1}/g,"");
            
            // for every line break, add a new entry to array
            var arr = strippedfeed.split("\n");
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
  
            // convert title array to string
            var title = titlearray.join("\n");
            // convert paragraph array to string
  
            console.log(title);
            console.log(paragrapharray);
            // convert paragraph array to string with newlines
            var paragraph = paragrapharray.join("\n");

            var decodedtitle = decode(title);
            var slicedtitle = decodedtitle.slice(8, decodedtitle.length);
            var embed = new Discord.MessageEmbed()
            .setTitle(slicedtitle)
            .setURL('https://www.bbc.com/news/live/world-europe-60542877')
            .setColor('#ff0000')
            .setThumbnail('https://ichef.bbci.co.uk/live-experience/cps/1024/cpsprodpb/2713/production/_123430001_hero_1133070132_rc2hrs97q98f_rtrmadp_3_ukraine-crisis-kyiv.jpg')
            .setFooter('BREAKING NEWS')
            .setTimestamp()
            .setDescription(decode(paragraph));
            client.user.setPresence({
              status: 'dnd',
            });
            client.user.setActivity(slicedtitle, {
              type: 'WATCHING'
            });
            client.channels.cache.get('946504863154589718').send('@everyone');
            client.channels.cache.get('946504863154589718').send({ embeds: [embed] });
        }
    });
  }, 20000);

});


client.login(process.env.TOKEN);