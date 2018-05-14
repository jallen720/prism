#include <cstdio>
#include <yaml.h>
#include "utils/yaml.h"

namespace game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class YAML_NODE_TYPE
{
    SCALAR,
    MAP,
    SEQUENCE,
};

struct YAML_NODE
{
    YAML_NODE_TYPE node_type;
    char * key;

    // Scalar data
    char * value;

    YAML_NODE * children[256];
    int child_count;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const char * const YAML_NODE_TYPE_NAMES[] =
{
    "SCALAR",
    "MAP",
    "SEQUENCE",
};

static const char * const YAML_TOKEN_TYPE_NAMES[] =
{
    "YAML_NO_TOKEN",
    "YAML_STREAM_START_TOKEN",
    "YAML_STREAM_END_TOKEN",
    "YAML_VERSION_DIRECTIVE_TOKEN",
    "YAML_TAG_DIRECTIVE_TOKEN",
    "YAML_DOCUMENT_START_TOKEN",
    "YAML_DOCUMENT_END_TOKEN",
    "YAML_BLOCK_SEQUENCE_START_TOKEN",
    "YAML_BLOCK_MAPPING_START_TOKEN",
    "YAML_BLOCK_END_TOKEN",
    "YAML_FLOW_SEQUENCE_START_TOKEN",
    "YAML_FLOW_SEQUENCE_END_TOKEN",
    "YAML_FLOW_MAPPING_START_TOKEN",
    "YAML_FLOW_MAPPING_END_TOKEN",
    "YAML_BLOCK_ENTRY_TOKEN",
    "YAML_FLOW_ENTRY_TOKEN",
    "YAML_KEY_TOKEN",
    "YAML_VALUE_TOKEN",
    "YAML_ALIAS_TOKEN",
    "YAML_ANCHOR_TOKEN",
    "YAML_TAG_TOKEN",
    "YAML_SCALAR_TOKEN",
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static YAML_NODE * create_yaml_node(YAML_NODE_TYPE node_type, const char * key, const char * value);
static void free_yaml(YAML_NODE * yaml);
static const yaml_token_t * process_map(YAML_NODE * map, const yaml_token_t * current_token);
static const yaml_token_t * process_sequence(YAML_NODE * sequence, const yaml_token_t * current_token);
static const yaml_token_t * process_child(YAML_NODE * parent, const char * key, const yaml_token_t * current_token);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Debugging
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void print_yaml(const YAML_NODE * yaml, int tab_level);
static const char * yaml_token_type_name(yaml_token_type_t token_type);
static const char * yaml_node_type_name(YAML_NODE_TYPE node_type);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void read_yaml_file(const char * path)
{
    // Initialize parser.
    yaml_parser_t parser;

    if(!yaml_parser_initialize(&parser))
    {
        fputs("Failed to initialize parser!\n", stderr);
    }

    // Initialize file handle.
    FILE * file_handle = fopen(path, "r");

    if(file_handle == NULL)
    {
        fputs("Failed to open file!\n", stderr);
    }

    // Set file to parse.
    yaml_parser_set_input_file(&parser, file_handle);

    // Get tokens.
    static const int MAX_TOKEN_COUNT = 256;
    yaml_token_t tokens[MAX_TOKEN_COUNT];
    yaml_token_t * current_token;
    yaml_token_type_t current_token_type;
    int token_count = 0;

    do
    {
        if(token_count >= MAX_TOKEN_COUNT)
        {
            fprintf(stderr, "token count in %s exceeds maximum token count of %d\n", path, MAX_TOKEN_COUNT);
            exit(1);
        }

        current_token = tokens + token_count++;
        yaml_parser_scan(&parser, current_token);
    }
    while(current_token->type != YAML_STREAM_END_TOKEN);

    // Cleanup.
    yaml_parser_delete(&parser);
    fclose(file_handle);

    // Print tokens.
    for(int i = 0; i < token_count; i++)
    {
        current_token = &tokens[i];
        current_token_type = current_token->type;
        printf("%s", yaml_token_type_name(current_token_type));

        if(current_token_type == yaml_token_type_t::YAML_SCALAR_TOKEN)
        {
            printf(" -------------> %s", current_token->data.scalar.value);
        }

        puts("");
    }

    puts("");

    // Process YAML.
    static const int ROOT_MAP_START_INDEX = 1;
    YAML_NODE * root = create_yaml_node(YAML_NODE_TYPE::MAP, "root", NULL);
    process_map(root, tokens + ROOT_MAP_START_INDEX);
    print_yaml(root, 0);

    // Cleanup
    for(int i = 0; i < token_count; i++)
    {
        yaml_token_delete(tokens + i);
    }

    free_yaml(root);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static YAML_NODE * create_yaml_node(YAML_NODE_TYPE node_type, const char * key, const char * value)
{
    YAML_NODE * yaml = (YAML_NODE *)malloc(sizeof(YAML_NODE));
    yaml->node_type = node_type;
    yaml->key = NULL;
    yaml->value = NULL;
    yaml->child_count = 0;

    if(key != NULL)
    {
        strcpy(yaml->key = (char *)malloc(strlen(key) + 1), key);
    }

    if(value != NULL)
    {
        strcpy(yaml->value = (char *)malloc(strlen(value) + 1), value);
    }

    return yaml;
}

static void free_yaml(YAML_NODE * yaml)
{
    char * yaml_key = yaml->key;

    if(yaml_key != NULL)
    {
        free(yaml_key);
    }

    switch(yaml->node_type)
    {
        case YAML_NODE_TYPE::SCALAR:
        {
            free(yaml->value);
            break;
        }
        case YAML_NODE_TYPE::MAP:
        case YAML_NODE_TYPE::SEQUENCE:
        {
            YAML_NODE ** children = yaml->children;

            for(int i = 0; i < yaml->child_count; i++)
            {
                free_yaml(children[i]);
            }

            break;
        }
        default:
        {
            fprintf(stderr, "unhandled yaml type %s when deleting yaml\n", yaml_node_type_name(yaml->node_type));
            exit(1);
        }
    }

    free(yaml);
}

static const yaml_token_t * process_map(YAML_NODE * map, const yaml_token_t * current_token)
{
    if(current_token->type != yaml_token_type_t::YAML_BLOCK_MAPPING_START_TOKEN)
    {
        fprintf(
            stderr,
            "first token in map block should be %s; was %s\n",
            yaml_token_type_name(yaml_token_type_t::YAML_BLOCK_MAPPING_START_TOKEN),
            yaml_token_type_name(current_token->type));

        exit(1);
    }

    // Skip YAML_BLOCK_MAPPING_START_TOKEN.
    current_token++;

    // Process fields in map until YAML_BLOCK_END_TOKEN is reached.
    while(current_token->type != yaml_token_type_t::YAML_BLOCK_END_TOKEN)
    {
        // Skip YAML_KEY_TOKEN.
        current_token++;

        // Store key value for initalizing child.
        const char * key = (const char *)current_token->data.scalar.value;

        // Skip YAML_VALUE_TOKEN.
        current_token += 2;

        // If the token after YAML_VALUE_TOKEN is YAML_KEY_TOKEN or YAML_BLOCK_END_TOKEN then no value was given.
        if(current_token->type == yaml_token_type_t::YAML_KEY_TOKEN
            || current_token->type == yaml_token_type_t::YAML_BLOCK_END_TOKEN)
        {
            fprintf(stderr, "no value for key '%s' in map '%s'\n", key, map->key);
            exit(1);
        }

        // Process child and store in map.
        current_token = process_child(map, key, current_token);
    }

    // yaml_token_delete(&token);
    return current_token;
}

static const yaml_token_t * process_sequence(YAML_NODE * sequence, const yaml_token_t * current_token)
{
    // Skip YAML_FLOW_SEQUENCE_START_TOKEN.
    current_token++;

    // Ensure sequence doesn't start with a comma.
    if(current_token->type == yaml_token_type_t::YAML_FLOW_ENTRY_TOKEN)
    {
        fprintf(stderr, "comma at start of sequence in sequence '%s'\n", sequence->key);
        exit(1);
    }

    // Process fields in map until YAML_BLOCK_END_TOKEN is reached.
    while(current_token->type != yaml_token_type_t::YAML_FLOW_SEQUENCE_END_TOKEN)
    {
        // Process child and store in sequence.
        current_token = process_child(sequence, NULL, current_token);

        // Skip YAML_FLOW_ENTRY_TOKEN.
        if(current_token->type == yaml_token_type_t::YAML_FLOW_ENTRY_TOKEN)
        {
            current_token++;

            if(current_token->type == yaml_token_type_t::YAML_FLOW_ENTRY_TOKEN)
            {
                fprintf(stderr, "multiple consecutive commas in sequence '%s'\n", sequence->key);
                exit(1);
            }
        }
    }

    return current_token;
}

static const yaml_token_t * process_child(YAML_NODE * parent, const char * key, const yaml_token_t * current_token)
{
    YAML_NODE * child;

    // Initialize child.
    if(current_token->type == yaml_token_type_t::YAML_SCALAR_TOKEN)
    {
        // Node is scalar.
        child = create_yaml_node(YAML_NODE_TYPE::SCALAR, key, (const char *)current_token->data.scalar.value);
    }
    else if(current_token->type == yaml_token_type_t::YAML_BLOCK_MAPPING_START_TOKEN)
    {
        // Node is map.
        child = create_yaml_node(YAML_NODE_TYPE::MAP, key, NULL);
        current_token = process_map(child, current_token);
    }
    else if(current_token->type == yaml_token_type_t::YAML_FLOW_SEQUENCE_START_TOKEN)
    {
        // Node is sequence.
        child = create_yaml_node(YAML_NODE_TYPE::SEQUENCE, key, NULL);
        current_token = process_sequence(child, current_token);
    }
    else
    {
        fprintf(
            stderr,
            "unsupported token type %s in node '%s'\n",
            yaml_token_type_name(current_token->type),
            parent->key);

        exit(1);
    }

    // Store child pointer in parent map's children array.
    parent->children[parent->child_count++] = child;

    // Return next token after child.
    return ++current_token;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Debugging
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void print_yaml(const YAML_NODE * yaml, int tab_level)
{
    static const char tab[] = "    ";
    const YAML_NODE_TYPE node_type = yaml->node_type;

    const bool isBlock =
        node_type == YAML_NODE_TYPE::MAP ||
        node_type == YAML_NODE_TYPE::SEQUENCE;

    if(isBlock)
    {
        puts("");
    }

    for(int i = 0; i < tab_level; i++)
    {
        printf("%s", tab);
    }

    if(yaml->key != NULL)
    {
        printf("%s: ", yaml->key);
    }

    if(isBlock)
    {
        puts("");

        for(int i = 0; i < yaml->child_count; i++)
        {
            print_yaml(yaml->children[i], tab_level + 1);
        }
    }
    else if(node_type == YAML_NODE_TYPE::SCALAR)
    {
        printf("%s\n", yaml->value);
    }
}

static const char * yaml_token_type_name(yaml_token_type_t token_type)
{
    return YAML_TOKEN_TYPE_NAMES[(int)token_type];
}

static const char * yaml_node_type_name(YAML_NODE_TYPE node_type)
{
    return YAML_NODE_TYPE_NAMES[(int)node_type];
}


} // namespace game
