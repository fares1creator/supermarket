#ifndef SM_HEADER_H
#define SM_HEADER_H

// ============================================================================
// STANDARD LIBRARY INCLUDES
// ============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// ============================================================================
// CONSTANTS AND MACROS
// ============================================================================
#define PRODUCTS_FILE "products.dat"
#define SALES_FILE "sales.dat"
#define INDEX_FILE "product_index.dat"
#define USERS_FILE "users.dat"
#define BACKUP_DIR "backups/"
#define LOW_STOCK_THRESHOLD 10

// ============================================================================
// DATA STRUCTURES
// ============================================================================

typedef struct {
    int productID;
    char name[50];
    float unitPrice;
    int quantityInStock;
    int isActive; // For soft delete (1 = active, 0 = deleted)
    char category[30];
    float discountPercent; // For promotions (0.0 to 100.0)
} Product;

typedef struct {
    int saleID;
    int productID;
    int quantitySold;
    float unitPrice;
    float discountApplied;
    float totalPrice;
    char date[11]; // YYYY-MM-DD format
    char time[9];  // HH:MM:SS format
} Sale;

typedef struct {
    int productID;
    long filePosition;
} ProductIndex;

typedef struct {
    char username[30];
    char password[50];
    int isAdmin;
} User;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
extern Product *productCache;
extern ProductIndex *indexCache;
extern int productCount;
extern int indexCount;
extern int nextProductID;
extern int nextSaleID;
extern int isLoggedIn;
extern int isAdmin;
extern char currentUser[30];

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

// Authentication System
void initializeUsers(void);
int authenticateUser(char *username, char *password);
void loginSystem(void);
void addUser(void);

// File Management & System Initialization
void initializeSystem(void);
void loadProductsToCache(void);
void saveProductsFromCache(void);
void loadIndexToCache(void);
void updateProductIndex(void);

// Backup & Restore System
void createBackup(void);
void restoreBackup(void);

// CSV Export Functions
void exportProductsToCSV(void);
void exportSalesToCSV(void);
void exportLowStockToCSV(void);

// Inventory Management (CRUD Operations)
void addProduct(void);
void viewAllProducts(void);
int findProductByID(int productID);
void updateProduct(void);
void deleteProduct(void);

// Discount & Promotion System
void setProductDiscount(void);
void viewPromotions(void);

// Sales Management
void getCurrentDateTime(char *dateStr, char *timeStr);
void processNewSale(void);
void viewAllSales(void);

// Search & Sort Functions
void searchProductByName(void);
void searchByCategory(void);
void sortProductsByPrice(void);

// Reports & Analytics
void lowStockReport(void);
void dailySalesReport(void);
void monthlyReport(void);
void bestSellingProducts(void);
void dateRangeReport(void);
void categoryReport(void);

// Menu Functions
void exportMenu(void);
void inventoryMenu(void);
void salesMenu(void);
void reportsMenu(void);
void adminMenu(void);
void mainMenu(void);

#endif // SM_HEADER_H