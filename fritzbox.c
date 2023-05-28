#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <openssl/md5.h>
#include "fritzbox.h"

#define HTTP_STATUS_OK 200

typedef struct
{
  const char *name;
  const char *id;
} fb_template_t;

typedef struct
{
  const char *name;
  const char *id;
  int8_t state;
  int8_t level;
  int8_t color;
} fb_device_t;

struct fritzbox
{
  const char *hostname;
  const char *user;
  const char *password;
  const char *base_url;
  CURL *curl;
  char *query_url;
  int number_of_templates;
  fb_template_t *templates;
  int number_of_devices;
  fb_device_t *devices;
};

typedef struct
{
  char *string;
  size_t size;
} curl_memory_t;

static size_t curl_callback(void *contents, size_t size, size_t nmemb, void *result)
{
  size *= nmemb;
  curl_memory_t *mem = (curl_memory_t*)result;
  mem->string = realloc(mem->string, mem->size + size + 1);
  assert(mem->string);
  memcpy(mem->string + mem->size, contents, size);
  mem->size += size;
  mem->string[mem->size] = '\0';
  return size;
}

static char *curl_query_url(CURL *curl, const char *url)
{
  assert(curl);
  assert(url);

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,curl_callback);
  curl_memory_t result = { NULL, 0 };
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
  CURLcode ret = curl_easy_perform(curl);
  if (ret != CURLE_OK)
    return NULL;
  if (!result.size)
    return NULL;

  long status;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
  if (status != HTTP_STATUS_OK)
  {
    free(result.string);
    return NULL;
  }

  return result.string;
}

static xmlNodePtr xml_find_child(xmlNodePtr root, xmlChar *name)
{
  for (xmlNodePtr node = root->xmlChildrenNode; node; node = node->next)
    if (!xmlStrcmp(node->name, name))
      return node;
  return NULL;
}

static const char *challenge_response(const char *challenge, const char* password)
{
  int len = (strlen(challenge) + 1 + strlen(password)) * 2;
  unsigned char utf16[len];
  char *dst = (char*)utf16;
  for (const char *src = challenge; *src; src++)
  {
    *dst++ = *src;
    *dst++ = '\0';
  }
  *dst++ = '-';
  *dst++ = '\0';
  for (const char *src = password; *src; src++)
  {
    *dst++ = *src;
    *dst++ = '\0';
  }

  char hash[MD5_DIGEST_LENGTH + 1];
  MD5(utf16, len, (unsigned char*)hash);

  static char response[2 * MD5_DIGEST_LENGTH + 1];
  for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
  {
    int high = (unsigned char)hash[i] >> 4;
    int low = (unsigned char)hash[i] & 0xf;

    response[2 * i] = high <= 9 ? '0' + high : 'a' + high - 10;
    response[2 * i + 1] =  low <= 9 ? '0' + low : 'a' + low - 10;
  }
  response[2 * MD5_DIGEST_LENGTH] = '\0';
  return response;
}

static int fb_device_index(fritzbox_t fb, const char *device_name)
{
  for (int index = 0; index < fb->number_of_devices; index++)
    if (!strcmp(device_name, fb->devices[index].name))
      return index;
  return FB_FAILURE;
}

static const char *itoa(int i)
{
  assert(i >= 0 && i <= 999);

  static char a[4] = "000";
  char *p = a + 3;
  do
  {
    *--p = i % 10 + '0';
    i /= 10;
  } while (i);

  return p;
}

fritzbox_t fb_create(const char *hostname, const char *user, const char *password)
{
  assert(hostname);
  assert(user);
  assert(password);

  fritzbox_t fb = calloc(1, sizeof(struct fritzbox));
  assert(fb);

  fb->user = strdup(user);
  assert(fb->user);

  fb->password = strdup(password);
  assert(fb->password);
  
  fb->hostname = strdup(hostname);
  assert(hostname);

  fb->curl = curl_easy_init();
  assert(fb->curl);

  xmlInitParser();
  
  return fb;
}

int fb_get_sid(fritzbox_t fb)
{
  assert(fb);

  int length1 = 7 + strlen(fb->hostname) + 14 + 1;
  char url1[length1];
  strcpy(url1, "http://");
  strcat(url1, fb->hostname);
  strcat(url1, "/login_sid.lua");

  char *result1 = curl_query_url(fb->curl, url1);
  if (!result1)
    return FB_FAILURE;

  xmlDocPtr doc1 = xmlReadMemory(result1, strlen(result1), "noname.xml", NULL, 0);
  assert(doc1);
  xmlNodePtr root1 = xmlDocGetRootElement(doc1);
  xmlNodePtr node1 = xml_find_child(root1, (xmlChar*)"Challenge");
  assert(node1);
  char *challenge = (char*)xmlNodeGetContent(node1);
  assert(challenge);
  const char *response = challenge_response(challenge, fb->password);

  int length2 = 7 + strlen(fb->hostname) + 24 + strlen(fb->user)
    + 10 + strlen(challenge) + 1 + strlen(response) + 1;
  char url2[length2];
  strcpy(url2, "http://");
  strcat(url2, fb->hostname);
  strcat(url2, "/login_sid.lua?username=");
  strcat(url2, fb->user);
  strcat(url2, "&response=");
  strcat(url2, challenge);
  strcat(url2, "-");
  strcat(url2, response);

  xmlFreeDoc(doc1);
  free(result1);

  char *result2 = curl_query_url(fb->curl, url2);
  if (!result2)
    return FB_FAILURE;
  
  xmlDocPtr doc2 = xmlReadMemory(result2, strlen(result2), "noname.xml", NULL, 0);
  assert(doc2);
  xmlNodePtr root2 = xmlDocGetRootElement(doc2);
  xmlNodePtr node2 = xml_find_child(root2, (xmlChar*)"SID");
  assert(node2);
  const char *sid = (char*)xmlNodeGetContent(node2);

  if (!strcmp(sid, "0000000000000000"))
  {
    xmlFreeDoc(doc2);
    free(result2);
    return FB_FAILURE;
  }

  free(fb->query_url);
  int length3 = 7 + strlen(fb->hostname) + 36 + strlen(sid) + 1;
  fb->query_url = malloc(length3);
  strcpy(fb->query_url, "http://");
  strcat(fb->query_url, fb->hostname);
  strcat(fb->query_url, "/webservices/homeautoswitch.lua?sid=");
  strcat(fb->query_url, sid);

  xmlFreeDoc(doc2);
  free(result2);

  return FB_SUCCESS;
}

void fb_free(fritzbox_t fb)
{
  assert(fb);

  free((void*)fb->hostname);
  free((void*)fb->user);
  free((void*)fb->password);
  free(fb->query_url);
  curl_easy_cleanup(fb->curl);
  free(fb->templates);
  free(fb->devices);
  free(fb);
  xmlCleanupParser();
}

int fb_get_templates(fritzbox_t fb)
{
  assert(fb);

  int length1 = strlen(fb->query_url) + 31 + 1;
  char url[length1];
  strcpy(url, fb->query_url);
  strcat(url, "&switchcmd=gettemplatelistinfos");

  char *result = curl_query_url(fb->curl, url);
  if (!result)
    return FB_FAILURE;

  xmlDocPtr doc = xmlReadMemory(result, strlen(result), "noname.xml", NULL, 0);
  assert(doc);
  xmlNodePtr root = xmlDocGetRootElement(doc);
  assert(root);

  int number_of_templates = 0;
  int length2 = 0;
  for (xmlNodePtr node = root->xmlChildrenNode; node; node = node->next)
  {
    xmlNodePtr name_node = xml_find_child(node, (xmlChar*)"name");
    const char *name = (char*)xmlNodeGetContent(name_node);
    const char *id = (char*)xmlGetProp(node, (xmlChar*)"identifier");

    number_of_templates++;
    length2 += strlen(name) + 1 + strlen(id) + 1;
  }

  fb_template_t *templates = malloc(number_of_templates * sizeof(fb_template_t) + length2);
  int index = 0;
  char *string = (char*)templates + number_of_templates * sizeof(fb_template_t);
  for (xmlNodePtr node = root->xmlChildrenNode; node; node = node->next)
  {
    xmlNodePtr name_node = xml_find_child(node, (xmlChar*)"name");
    const char *name = (char*)xmlNodeGetContent(name_node);
    const char *id = (char*)xmlGetProp(node, (xmlChar*)"identifier");

    strcpy(string, name);
    templates[index].name = string;
    string += strlen(name) + 1;
    strcpy(string, id);
    templates[index].id = string;
    string += strlen(id) + 1;
    index++;
  }

  xmlFreeDoc(doc);
  free(result);
  free(fb->templates);
  fb->templates = templates;
  fb->number_of_templates = number_of_templates;

  return FB_SUCCESS;
}

int fb_apply_template(fritzbox_t fb, const char *template_name)
{
  assert(fb);
  assert(template_name);

  int index;
  for (index = 0; index < fb->number_of_templates; index++)
    if (!strcmp(template_name, fb->templates[index].name))
      break;
  if (index >= fb->number_of_templates)
    return FB_FAILURE;
  
  int length = strlen(fb->query_url) + 29 + strlen(fb->templates[index].id) + 1;
  char url[length];
  strcpy(url, fb->query_url);
  strcat(url, "&switchcmd=applytemplate&ain=");
  strcat(url, fb->templates[index].id);

  char *result = curl_query_url(fb->curl, url);
  if (!result)
    return FB_FAILURE;

  free(result);
  return FB_SUCCESS;
}

int fb_get_devices(fritzbox_t fb)
{
  assert(fb);

  int length1 = strlen(fb->query_url) + 29 + 1;
  char url[length1];
  strcpy(url, fb->query_url);
  strcat(url, "&switchcmd=getdevicelistinfos");
  
  char *result = curl_query_url(fb->curl, url);
  if (!result)
    return FB_FAILURE;

  xmlDocPtr doc = xmlReadMemory(result, strlen(result), "noname.xml", NULL, 0);
  assert(doc);
  xmlNodePtr root = xmlDocGetRootElement(doc);
  assert(root);

  int number_of_devices = 0;
  int length2 = 0;
  for (xmlNodePtr node = root->xmlChildrenNode; node; node = node->next)
  {
    xmlNodePtr present_node = xml_find_child(node, (xmlChar*)"present");
    const char *present = (char*)xmlNodeGetContent(present_node);
    if (strcmp(present, "1"))
      continue;

    xmlNodePtr state_node = xml_find_child(node, (xmlChar*)"simpleonoff");
    if (!state_node)
      continue;

    xmlNodePtr name_node = xml_find_child(node, (xmlChar*)"name");
    const char *name = (char*)xmlNodeGetContent(name_node);
    const char *id = (char*)xmlGetProp(node, (xmlChar*)"identifier");

    number_of_devices++;
    length2 += strlen(name) + 1 + strlen(id) + 2 + 1;
  }

  fb_device_t *devices = malloc(number_of_devices * sizeof(fb_device_t) + length2);
  int index = 0;
  char *string = (char*)devices + number_of_devices * sizeof(fb_device_t);
  for (xmlNodePtr node = root->xmlChildrenNode; node; node = node->next)
  {
    xmlNodePtr present_node = xml_find_child(node, (xmlChar*)"present");
    const char *present = (char*)xmlNodeGetContent(present_node);
    if (strcmp(present, "1"))
      continue;

    xmlNodePtr state_node = xml_find_child(node, (xmlChar*)"simpleonoff");
    if (!state_node)
      continue;

    xmlNodePtr name_node = xml_find_child(node, (xmlChar*)"name");
    const char *name = (char*)xmlNodeGetContent(name_node);
    const char *id = (char*)xmlGetProp(node, (xmlChar*)"identifier");

    uint8_t state = strcmp((char*)xmlNodeGetContent(state_node->xmlChildrenNode), "1") == 0;

    int8_t level = FB_FAILURE;
    int8_t color = FB_FAILURE;
    xmlNodePtr level_node = xml_find_child(node, (xmlChar*)"levelcontrol");
    if (level_node)
    {
      xmlNodePtr percentage_node = xml_find_child(level_node, (xmlChar*)"levelpercentage");
      assert(percentage_node);
      level = atoi((char*)xmlNodeGetContent(percentage_node));
      
      xmlNodePtr color_node = xml_find_child(node, (xmlChar*)"colorcontrol");
      assert(color_node);
      xmlNodePtr hue_node = xml_find_child(color_node, (xmlChar*)"hue");
      assert(hue_node);
      const char *hue = (char*)xmlNodeGetContent(hue_node);
      xmlNodePtr temperature_node = xml_find_child(color_node, (xmlChar*)"temperature");
      assert(temperature_node);
      const char *temperature = (char*)xmlNodeGetContent(temperature_node);
      assert(*hue || *temperature);
      int value = atoi(hue) + atoi(temperature);
      switch (value)
      {
      case 358:
	color = FB_RED;
	break;
      case 35:
	color = FB_ORANGE;
	break;
      case 52:
	color = FB_YELLOW;
	break;
      case 92:
	color = FB_CHARTREUSE;
	break;
      case 120:
	color = FB_GREEN;
	break;
      case 160:
	color = FB_TURQUOISE;
	break;
      case 195:
	color = FB_CYAN;
	break;
      case 212:
	color = FB_AZURE;
	break;
      case 225:
	color = FB_BLUE;
	break;
      case 266:
	color = FB_PURPLE;
	break;
      case 296:
	color = FB_MAGENTA;
	break;
      case 335:
	color = FB_PINK;
	break;
      case 2700:
	color = FB_WHITE_2700;
	break;
      case 3000:
	color = FB_WHITE_3000;
	break;
      case 3400:
	color = FB_WHITE_3400;
	break;
      case 3800:
	color = FB_WHITE_3800;
	break;
      case 4200:
	color = FB_WHITE_4200;
	break;
      case 4700:
	color = FB_WHITE_4700;
	break;
      case 5300:
	color = FB_WHITE_5300;
	break;
      case 5900:
	color = FB_WHITE_5900;
	break;
      case 6500:
	color = FB_WHITE_6500;
	break;
      default:
	assert(0);
      }
    }
    
    strcpy(string, name);
    devices[index].name = string;
    string += strlen(name) + 1;

    int length = strlen(id);
    char *space = strchr(id, ' ');
    assert(space);
    *space++ = '\0';
    strcpy(string, id);
    strcat(string, "%20");
    strcat(string, space);
    space = strchr(space, ' ');
    assert(!space);
    devices[index].id = string;
    string += length + 2 + 1;

    devices[index].state = state;
    devices[index].level = level;
    devices[index].color = color;
    index++;
  }
  
  xmlFreeDoc(doc);
  free(result);
  free(fb->devices);
  fb->devices = devices;
  fb->number_of_devices = number_of_devices;

  return FB_SUCCESS;
}

int fb_get_state(fritzbox_t fb, const char *device_name)
{
  assert(fb);
  assert(device_name);

  int index = fb_device_index(fb, device_name);
  if (index < 0)
    return FB_FAILURE;

  return fb->devices[index].state;
}

int fb_set_state(fritzbox_t fb, const char *device_name, int state)
{
  assert(fb);
  assert(device_name);

  int index = fb_device_index(fb, device_name);
  if (index < 0)
    return FB_FAILURE;
  if (fb->devices[index].state == state)
    return FB_SUCCESS;

  int length = strlen(fb->query_url) + 28 + strlen(fb->devices[index].id) + 1;
  char url[length];
  strcpy(url, fb->query_url);
  if (state)
    strcat(url, "&switchcmd=setswitchon&ain=");
  else
    strcat(url, "&switchcmd=setswitchoff&ain=");
  strcat(url, fb->devices[index].id);

  char *result = curl_query_url(fb->curl, url);
  if (!result)
    return FB_FAILURE;

  free(result);

  fb->devices[index].state = state;
  return FB_SUCCESS;
}

int fb_get_level(fritzbox_t fb, const char *device_name)
{
  assert(fb);
  assert(device_name);

  int index = fb_device_index(fb, device_name);
  if (index < 0)
    return FB_FAILURE;

  return fb->devices[index].level;
}

int fb_set_level(fritzbox_t fb, const char *device_name, int level)
{
  assert(fb);
  assert(device_name);
  assert(level >= 0 && level <= 100);

  int index = fb_device_index(fb, device_name);
  if (index < 0)
    return FB_FAILURE;
  if (fb->devices[index].level < 0)
    return FB_FAILURE;
  if (fb->devices[index].level == level)
    return FB_SUCCESS;

  int length = strlen(fb->query_url) + 34 + strlen(fb->devices[index].id) + 7 + 3 + 1;
  char url[length];
  strcpy(url, fb->query_url);
  strcat(url, "&switchcmd=setlevelpercentage&ain=");
  strcat(url, fb->devices[index].id);
  strcat(url, "&level=");
  strcat(url, itoa(level));

  char *result = curl_query_url(fb->curl, url);
  if (!result)
    return FB_FAILURE;

  free(result);

  fb->devices[index].level = level;

  return FB_SUCCESS;
}

int fb_get_color(fritzbox_t fb, const char *device_name)
{
  assert(fb);
  assert(device_name);

  int index = fb_device_index(fb, device_name);
  if (index < 0)
    return FB_FAILURE;

  return fb->devices[index].color;

}

int fb_set_color(fritzbox_t fb, const char *device_name, int color)
{
  assert(fb);
  assert(device_name);
  assert(color >= FB_RED && color <= FB_WHITE_6500);

  int index = fb_device_index(fb, device_name);
  if (index < 0)
    return FB_FAILURE;
  if (fb->devices[index].color < 0)
    return FB_FAILURE;
  if (fb->devices[index].color == color)
    return FB_SUCCESS;

  // strlen(fb->query_url) + 24 + strlen(fb->devices[index].id) + 5 + 3 + 12 + 3 + 11 + 1
  // strlen(fb->query_url) + 35 + strlen(fb->devices[index].id) + 13 + 4 + 11 + 1
  int length = strlen(fb->query_url) + strlen(fb->devices[index].id) + 64;
  char url[length];
  strcpy(url, fb->query_url);
  if (color <= FB_PINK)
  {
    static const char *hue[] =
    {
      "358",
      "35",
      "52",
      "92",
      "120",
      "160",
      "195",
      "212",
      "225",
      "266",
      "296",
      "335",
    };
    static const char *saturation[] =
    {
      "180",
      "214",
      "153",
      "123",
      "160",
      "145",
      "179",
      "169",
      "204",
      "169",
      "140",
      "180",
    };
    strcat(url, "&switchcmd=setcolor&ain=");
    strcat(url, fb->devices[index].id);
    strcat(url, "&hue=");
    strcat(url, hue[color - FB_RED]);
    strcat(url, "&saturation=");
    strcat(url, saturation[color - FB_RED]);
  }
  else // color >= FB_WHITE_2700
  {
    static const char *temperature[] =
    {
      "2700",
      "3000",
      "3400",
      "3800",
      "4200",
      "4700",
      "5300",
      "5900",
      "6500",
    };
    strcat(url, "&switchcmd=setcolortemperature&ain=");
    strcat(url, fb->devices[index].id);
    strcat(url, "&temperature=");
    strcat(url, temperature[color - FB_WHITE_2700]);

  }
  strcat(url, "&duration=0");

  char *result = curl_query_url(fb->curl, url);
  if (!result)
    return FB_FAILURE;

  free(result);

  fb->devices[index].color = color;

  return FB_SUCCESS;
}
