---
layout: default
title: Home
---

<script>
function getQueryVariable(variable)
{
	var query = window.location.search.substring(1);
	var vars = query.split("&");
	for (var i=0;i<vars.length;i++) {
		var pair = vars[i].split("=");
		if(pair[0] == variable){return pair[1];}
	}
	return(false);
}

if ($name = getQueryVariable("name")) {
	window.location.href = "https://www.dockapps.net/" + $name;
}

</script>

An archive of
{{ site.posts.size | minus: site.categories.size }}
[Window Maker](http://windowmaker.org) dockapps.

categories
----------
{% assign sorted_cats = site.categories | sort %}
{% for category in sorted_cats %}
* [{{ category | first }} ({{ category | last | size }})](/category/{{ category | first }})
{% endfor %}

{% include_relative README.md %}
